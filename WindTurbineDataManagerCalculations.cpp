/**
 * WindTurbineDataManagerCalculations.cpp
 * Implementierung der fehlenden Berechnungsfunktionen für die WindTurbineDataManager-Klasse
 * 
 * Diese Datei enthält die Implementierungen der Funktionen, die für die Datenvisualisierung
 * und Diagrammgenerierung benötigt werden.
 */

#include "WindTurbineDataManager.h"
#include "WindTurbineConstants.h"
#include <ArduinoJson.h>

/**
 * Lädt die Experimentdaten als JSON-String
 * @param filename Der Dateiname des Experiments
 * @return JSON-String mit den Experimentdaten
 */
String WindTurbineDataManager::loadExperimentDataAsJSON(const char* filename) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei: " + String(filename));
    return "{}";
  }
  
  String jsonData = file.readString();
  file.close();
  
  return jsonData;
}

/**
 * Berechnet die Daten für das Main Effects Plot
 * @param filename Der Dateiname des Experiments
 * @param meanLow Array für die Mittelwerte bei niedrigem Faktorniveau (-1)
 * @param meanHigh Array für die Mittelwerte bei hohem Faktorniveau (+1)
 * @param overallMean Referenz für den Gesamtmittelwert
 * @param minResponse Referenz für den minimalen Antwortwert
 * @param maxResponse Referenz für den maximalen Antwortwert
 */
void WindTurbineDataManager::calculateMainEffectsData(const char* filename, float* meanLow, float* meanHigh, 
                                                     float& overallMean, float& minResponse, float& maxResponse) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei: " + String(filename));
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Fehler beim Parsen der JSON-Daten: " + String(error.c_str()));
    return;
  }
  
  // Teilfaktorielle Daten laden
  JsonArray tfMittelwerteArray = doc["teilfaktoriellMittelwerte"];
  
  // Gesamtmittelwert berechnen
  overallMean = 0;
  for (int i = 0; i < tfMittelwerteArray.size(); i++) {
    overallMean += tfMittelwerteArray[i].as<float>();
  }
  overallMean /= tfMittelwerteArray.size();
  
  // Initialisierung der Min/Max-Werte
  minResponse = tfMittelwerteArray[0].as<float>();
  maxResponse = tfMittelwerteArray[0].as<float>();
  
  // Für jeden Faktor
  for (int i = 0; i < 5; i++) {
    float summeNiedrig = 0;
    float summeHoch = 0;
    int anzahlNiedrig = 0;
    int anzahlHoch = 0;
    
    // Teilfaktorieller Plan
    JsonArray tfPlanArray = doc["teilfaktoriellPlan"];
    if (!tfPlanArray.isNull()) {
      // Wenn der Plan in den Daten vorhanden ist
      for (int j = 0; j < tfMittelwerteArray.size(); j++) {
        if (tfPlanArray[j][i] == -1) {
          summeNiedrig += tfMittelwerteArray[j].as<float>();
          anzahlNiedrig++;
        } else if (tfPlanArray[j][i] == 1) {
          summeHoch += tfMittelwerteArray[j].as<float>();
          anzahlHoch++;
        }
      }
    } else {
      // Fallback: Standardplan verwenden
      const int standardPlan[8][5] = {
        {-1, -1, -1, -1, 1},
        {1, -1, -1, 1, -1},
        {-1, 1, -1, 1, 1},
        {1, 1, -1, -1, -1},
        {-1, -1, 1, 1, -1},
        {1, -1, 1, -1, 1},
        {-1, 1, 1, -1, -1},
        {1, 1, 1, 1, 1}
      };
      
      for (int j = 0; j < 8 && j < tfMittelwerteArray.size(); j++) {
        if (standardPlan[j][i] == -1) {
          summeNiedrig += tfMittelwerteArray[j].as<float>();
          anzahlNiedrig++;
        } else if (standardPlan[j][i] == 1) {
          summeHoch += tfMittelwerteArray[j].as<float>();
          anzahlHoch++;
        }
      }
    }
    
    // Mittelwerte berechnen
    meanLow[i] = (anzahlNiedrig > 0) ? (summeNiedrig / anzahlNiedrig) : 0;
    meanHigh[i] = (anzahlHoch > 0) ? (summeHoch / anzahlHoch) : 0;
    
    // Min/Max aktualisieren
    if (meanLow[i] < minResponse) minResponse = meanLow[i];
    if (meanHigh[i] < minResponse) minResponse = meanHigh[i];
    if (meanLow[i] > maxResponse) maxResponse = meanLow[i];
    if (meanHigh[i] > maxResponse) maxResponse = meanHigh[i];
  }
  
  // Sicherheitsabstand für Min/Max
  float range = maxResponse - minResponse;
  minResponse -= range * 0.1;
  maxResponse += range * 0.1;
}

/**
 * Berechnet die Daten für das Pareto-Diagramm
 * @param filename Der Dateiname des Experiments
 * @param effects Array für die Effekte
 * @param sortedIndices Array für die sortierten Indizes
 * @param percentages Array für die Prozentanteile
 * @param cumulative Array für die kumulativen Prozentanteile
 */
void WindTurbineDataManager::calculateParetoData(const char* filename, float* effects, int* sortedIndices, 
                                               float* percentages, float* cumulative) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei: " + String(filename));
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Fehler beim Parsen der JSON-Daten: " + String(error.c_str()));
    return;
  }
  
  // Effekte laden
  JsonArray effekteArray = doc["effekte"];
  if (effekteArray.isNull() || effekteArray.size() == 0) {
    // Fallback: Beispieleffekte
    effects[0] = 0.5;
    effects[1] = -0.3;
    effects[2] = 0.2;
    effects[3] = -0.7;
    effects[4] = 0.4;
  } else {
    for (int i = 0; i < 5 && i < effekteArray.size(); i++) {
      effects[i] = effekteArray[i].as<float>();
    }
  }
  
  // Absolute Effekte für Sortierung
  float absEffects[5];
  for (int i = 0; i < 5; i++) {
    absEffects[i] = abs(effects[i]);
    sortedIndices[i] = i;
  }
  
  // Sortieren (Bubble-Sort) nach absoluten Effekten
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4 - i; j++) {
      if (absEffects[j] < absEffects[j + 1]) {
        // Effekte tauschen
        float tempEffect = absEffects[j];
        absEffects[j] = absEffects[j + 1];
        absEffects[j + 1] = tempEffect;
        
        // Indizes tauschen
        int tempIndex = sortedIndices[j];
        sortedIndices[j] = sortedIndices[j + 1];
        sortedIndices[j + 1] = tempIndex;
      }
    }
  }
  
  // Gesamtsumme der absoluten Effekte
  float totalEffect = 0;
  for (int i = 0; i < 5; i++) {
    totalEffect += absEffects[i];
  }
  
  // Prozentanteile und kumulative Prozente berechnen
  float cumulativeSum = 0;
  for (int i = 0; i < 5; i++) {
    percentages[i] = (totalEffect > 0) ? (absEffects[i] / totalEffect * 100.0) : 0;
    cumulativeSum += percentages[i];
    cumulative[i] = cumulativeSum;
  }
}

/**
 * Berechnet die Daten für das Interaction Plot
 * @param filename Der Dateiname des Experiments
 * @param factor1 Index des ersten Faktors
 * @param factor2 Index des zweiten Faktors
 * @param data Array für die Interaktionsdaten
 * @param dataAvailable Array für die Verfügbarkeit der Daten
 */
void WindTurbineDataManager::calculateInteractionData(const char* filename, int factor1, int factor2, 
                                                    float* data, bool* dataAvailable) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei: " + String(filename));
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Fehler beim Parsen der JSON-Daten: " + String(error.c_str()));
    return;
  }
  
  // Bestimme die beiden stärksten Faktoren, falls nicht angegeben
  if (factor1 == factor2) {
    JsonArray effekteArray = doc["effekte"];
    if (!effekteArray.isNull() && effekteArray.size() >= 5) {
      // Finde die beiden stärksten Faktoren
      float absEffekte[5];
      int indices[5] = {0, 1, 2, 3, 4};
      
      for (int i = 0; i < 5; i++) {
        absEffekte[i] = abs(effekteArray[i].as<float>());
      }
      
      // Sortieren
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4 - i; j++) {
          if (absEffekte[j] < absEffekte[j + 1]) {
            float temp = absEffekte[j];
            absEffekte[j] = absEffekte[j + 1];
            absEffekte[j + 1] = temp;
            
            int tempIdx = indices[j];
            indices[j] = indices[j + 1];
            indices[j + 1] = tempIdx;
          }
        }
      }
      
      factor1 = indices[0];
      factor2 = indices[1];
    } else {
      // Fallback
      factor1 = 0;
      factor2 = 1;
    }
  }
  
  // Initialisiere Daten als nicht verfügbar
  for (int i = 0; i < 4; i++) {
    dataAvailable[i] = false;
  }
  
  // Lade Daten aus vollfaktoriellen Messungen
  JsonArray vfMittelwerteArray = doc["vollfaktoriellMittelwerte"];
  JsonArray ausgewaehlteVollfaktorenArray = doc["ausgewaehlteVollfaktoren"];
  
  if (!vfMittelwerteArray.isNull() && !ausgewaehlteVollfaktorenArray.isNull()) {
    // Prüfe, ob die gewählten Faktoren im vollfaktoriellen Plan sind
    bool factor1InPlan = false;
    bool factor2InPlan = false;
    int factor1Index = -1;
    int factor2Index = -1;
    
    for (int i = 0; i < ausgewaehlteVollfaktorenArray.size(); i++) {
      int faktor = ausgewaehlteVollfaktorenArray[i].as<int>();
      if (faktor == factor1) {
        factor1InPlan = true;
        factor1Index = i;
      }
      if (faktor == factor2) {
        factor2InPlan = true;
        factor2Index = i;
      }
    }
    
    if (factor1InPlan && factor2InPlan) {
      // Beide Faktoren sind im Plan, wir können alle Datenpunkte extrahieren
      for (int i = 0; i < 8 && i < vfMittelwerteArray.size(); i++) {
        // Bestimme die Faktorstufen für diesen Versuch
        bool f1High = (i & (1 << factor1Index)) != 0;
        bool f2High = (i & (1 << factor2Index)) != 0;
        
        // Bestimme den Index im data-Array
        int dataIndex = (f1High ? 1 : 0) + (f2High ? 2 : 0);
        
        // Speichere den Wert
        data[dataIndex] = vfMittelwerteArray[i].as<float>();
        dataAvailable[dataIndex] = true;
      }
    }
  }
  
  // Für fehlende Datenpunkte: Interpolation oder Schätzung
  interpolateMissingData(data, dataAvailable);
}

/**
 * Interpoliert fehlende Datenpunkte für das Interaction Plot
 * @param data Array mit den Datenpunkten
 * @param available Array mit der Verfügbarkeit der Datenpunkte
 */
void WindTurbineDataManager::interpolateMissingData(float data[4], bool available[4]) {
  // Zähle verfügbare Datenpunkte
  int availableCount = 0;
  for (int i = 0; i < 4; i++) {
    if (available[i]) availableCount++;
  }
  
  if (availableCount == 4) {
    // Alle Daten vorhanden, nichts zu tun
    return;
  } else if (availableCount == 0) {
    // Keine Daten vorhanden, Standardwerte setzen
    data[0] = 0.5;
    data[1] = 0.7;
    data[2] = 0.6;
    data[3] = 0.9;
    for (int i = 0; i < 4; i++) {
      available[i] = true;
    }
    return;
  }
  
  // Interpolation basierend auf verfügbaren Datenpunkten
  if (!available[0] && available[1] && available[2] && available[3]) {
    // Fehlender Punkt: (-1, -1)
    data[0] = data[1] + data[2] - data[3];
    available[0] = true;
  } else if (available[0] && !available[1] && available[2] && available[3]) {
    // Fehlender Punkt: (+1, -1)
    data[1] = data[0] - data[2] + data[3];
    available[1] = true;
  } else if (available[0] && available[1] && !available[2] && available[3]) {
    // Fehlender Punkt: (-1, +1)
    data[2] = data[0] - data[1] + data[3];
    available[2] = true;
  } else if (available[0] && available[1] && available[2] && !available[3]) {
    // Fehlender Punkt: (+1, +1)
    data[3] = data[1] + data[2] - data[0];
    available[3] = true;
  } else if (availableCount == 2) {
    // Zwei Punkte fehlen, komplexere Interpolation
    if (available[0] && available[3]) {
      // Diagonale Punkte vorhanden
      float diff = (data[3] - data[0]) / 2;
      data[1] = data[0] + diff;
      data[2] = data[0] + diff;
      available[1] = true;
      available[2] = true;
    } else if (available[1] && available[2]) {
      // Andere Diagonale vorhanden
      float diff = (data[2] - data[1]) / 2;
      data[0] = data[1] - diff;
      data[3] = data[2] + diff;
      available[0] = true;
      available[3] = true;
    } else if (available[0] && available[1]) {
      // Obere Zeile vorhanden
      float diff = data[1] - data[0];
      data[2] = data[0] * 0.9;  // Leicht niedriger
      data[3] = data[1] * 1.1;  // Leicht höher
      available[2] = true;
      available[3] = true;
    } else if (available[2] && available[3]) {
      // Untere Zeile vorhanden
      float diff = data[3] - data[2];
      data[0] = data[2] * 0.9;  // Leicht niedriger
      data[1] = data[3] * 0.9;  // Leicht niedriger
      available[0] = true;
      available[1] = true;
    } else if (available[0] && available[2]) {
      // Linke Spalte vorhanden
      float diff = data[2] - data[0];
      data[1] = data[0] * 1.1;  // Leicht höher
      data[3] = data[2] * 1.1;  // Leicht höher
      available[1] = true;
      available[3] = true;
    } else if (available[1] && available[3]) {
      // Rechte Spalte vorhanden
      float diff = data[3] - data[1];
      data[0] = data[1] * 0.9;  // Leicht niedriger
      data[2] = data[3] * 0.9;  // Leicht niedriger
      available[0] = true;
      available[2] = true;
    }
  } else if (availableCount == 1) {
    // Nur ein Punkt vorhanden, einfache Schätzung
    float baseValue = 0;
    int baseIndex = -1;
    
    for (int i = 0; i < 4; i++) {
      if (available[i]) {
        baseValue = data[i];
        baseIndex = i;
        break;
      }
    }
    
    // Schätze andere Punkte basierend auf typischen Mustern
    for (int i = 0; i < 4; i++) {
      if (!available[i]) {
        if (i == 0) data[i] = baseValue * 0.8;  // (-1, -1) typischerweise niedriger
        else if (i == 1) data[i] = baseValue * 1.1;  // (+1, -1) leicht höher
        else if (i == 2) data[i] = baseValue * 1.1;  // (-1, +1) leicht höher
        else if (i == 3) data[i] = baseValue * 1.2;  // (+1, +1) typischerweise höher
        available[i] = true;
      }
    }
  }
}

/**
 * Prüft, ob vollständige Faktorialdaten für zwei Faktoren vorhanden sind
 * @param filename Der Dateiname des Experiments
 * @param factor1 Index des ersten Faktors
 * @param factor2 Index des zweiten Faktors
 * @return true, wenn vollständige Daten vorhanden sind
 */
bool WindTurbineDataManager::hasCompleteFactorialData(const char* filename, int factor1, int factor2) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    return false;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    return false;
  }
  
  // Prüfe, ob vollfaktorielle Daten vorhanden sind
  JsonArray vfMittelwerteArray = doc["vollfaktoriellMittelwerte"];
  JsonArray ausgewaehlteVollfaktorenArray = doc["ausgewaehlteVollfaktoren"];
  
  if (vfMittelwerteArray.isNull() || ausgewaehlteVollfaktorenArray.isNull()) {
    return false;
  }
  
  // Prüfe, ob beide Faktoren im vollfaktoriellen Plan sind
  bool factor1InPlan = false;
  bool factor2InPlan = false;
  
  for (int i = 0; i < ausgewaehlteVollfaktorenArray.size(); i++) {
    int faktor = ausgewaehlteVollfaktorenArray[i].as<int>();
    if (faktor == factor1) factor1InPlan = true;
    if (faktor == factor2) factor2InPlan = true;
  }
  
  return factor1InPlan && factor2InPlan;
}

/**
 * Validiert die Daten für ein bestimmtes Diagramm
 * @param chartType Der Typ des Diagramms
 * @param filename Der Dateiname des Experiments
 */
void WindTurbineDataManager::validateChartData(const String& chartType, const char* filename) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei: " + String(filename));
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Fehler beim Parsen der JSON-Daten: " + String(error.c_str()));
    return;
  }
  
  // Validierung je nach Diagrammtyp
  if (chartType == "main-effects") {
    // Prüfe, ob teilfaktorielle Daten vorhanden sind
    JsonArray tfMittelwerteArray = doc["teilfaktoriellMittelwerte"];
    if (tfMittelwerteArray.isNull() || tfMittelwerteArray.size() < 8) {
      Serial.println("Warnung: Unvollständige teilfaktorielle Daten für Main Effects Plot");
    }
  } else if (chartType == "pareto") {
    // Prüfe, ob Effekte vorhanden sind
    JsonArray effekteArray = doc["effekte"];
    if (effekteArray.isNull() || effekteArray.size() < 5) {
      Serial.println("Warnung: Unvollständige Effektdaten für Pareto-Diagramm");
    }
  } else if (chartType == "interaction") {
    // Prüfe, ob vollfaktorielle Daten vorhanden sind
    JsonArray vfMittelwerteArray = doc["vollfaktoriellMittelwerte"];
    if (vfMittelwerteArray.isNull() || vfMittelwerteArray.size() < 8) {
      Serial.println("Warnung: Unvollständige vollfaktorielle Daten für Interaction Plot");
    }
  }
}

/**
 * Gibt den Titel für ein bestimmtes Diagramm zurück
 * @param chartType Der Typ des Diagramms
 * @return Der Titel des Diagramms
 */
String WindTurbineDataManager::getChartTitle(const String& chartType) {
  if (chartType == "main-effects") {
    return "Main Effects Plot";
  } else if (chartType == "pareto") {
    return "Pareto-Diagramm";
  } else if (chartType == "interaction") {
    return "Interaction Plot";
  } else if (chartType == "effects") {
    return "Effekt-Diagramm";
  } else if (chartType == "factorial") {
    return "Faktorieller Vergleich";
  } else {
    return "Windkraft-Experiment Diagramm";
  }
}

/**
 * Generiert Metadaten für ein Diagramm
 * @param chartType Der Typ des Diagramms
 * @param filename Der Dateiname des Experiments
 * @return JSON-String mit den Metadaten
 */
String WindTurbineDataManager::generateChartMetadata(const String& chartType, const char* filename) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    return "{}";
  }
  
  DynamicJsonDocument docSource(8192);
  DeserializationError error = deserializeJson(docSource, file);
  file.close();
  
  if (error) {
    return "{}";
  }
  
  // Erstelle Metadaten-Dokument
  DynamicJsonDocument docMeta(1024);
  
  // Allgemeine Metadaten
  docMeta["chart_type"] = chartType;
  docMeta["filename"] = filename;
  docMeta["title"] = getChartTitle(chartType);
  docMeta["description"] = docSource["description"] | "Windkraft-Experiment";
  docMeta["timestamp"] = docSource["timestamp"] | "Unbekannt";
  docMeta["export_date"] = formatTimestamp();
  
  // Diagrammspezifische Metadaten
  if (chartType == "main-effects") {
    JsonArray effekteArray = docSource["effekte"];
    if (!effekteArray.isNull()) {
      JsonArray effekteMeta = docMeta.createNestedArray("effects");
      for (int i = 0; i < 5 && i < effekteArray.size(); i++) {
        effekteMeta.add(effekteArray[i].as<float>());
      }
    }
  } else if (chartType == "pareto") {
    // Berechne Pareto-Daten für Metadaten
    float effects[5];
    int sortedIndices[5];
    float percentages[5];
    float cumulative[5];
    
    calculateParetoData(filename, effects, sortedIndices, percentages, cumulative);
    
    JsonArray paretoMeta = docMeta.createNestedArray("pareto_data");
    for (int i = 0; i < 5; i++) {
      JsonObject item = paretoMeta.createNestedObject();
      item["factor"] = sortedIndices[i];
      item["effect"] = effects[sortedIndices[i]];
      item["percentage"] = percentages[i];
      item["cumulative"] = cumulative[i];
    }
  }
  
  // Serialisiere zu String
  String metadataJson;
  serializeJson(docMeta, metadataJson);
  
  return metadataJson;
}

/**
 * Formatiert einen Zeitstempel
 * @return Formatierter Zeitstempel als String
 */
String WindTurbineDataManager::formatTimestamp() {
  // Einfache Implementierung ohne Zeitzone
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  
  char buffer[20];
  sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  
  return String(buffer);
}
