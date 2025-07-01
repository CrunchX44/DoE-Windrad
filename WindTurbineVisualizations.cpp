/**
 * WindTurbineVisualizations.cpp
 * KORRIGIERTE Visualisierungsfunktionen für das Windkraftanlagen-Experiment
 * 
 * BEHOBENE FEHLER:
 * - Doppelte Deklaration von y_range in zeigeInteraktionsDiagramm() entfernt
 * - Mathematische Korrektheit aller Diagramme überprüft und verbessert
 * - Konsistente Float-Berechnungen für präzise Positionierung
 */

#include "WindTurbineExperiment.h"

/**
 * Zeigt ein Haupteffekte-Diagramm (Main Effects Plot) an
 * Berechnet Mittelwerte für -1 und +1 Level aus den teilfaktoriellen Daten
 */
void WindTurbineExperiment::zeigeHaupteffekteDiagramm() {
  // Diagrammbereich definieren
  int startX = 40;
  int startY = 80;
  int plotWidth = 400;
  int plotHeight = 180;
  int factorHeight = plotHeight / 3;
  int factorWidth = plotWidth / 3;
  
  // Hauptrahmen für das gesamte Diagramm
  tft.drawRoundRect(startX-10, startY-10, plotWidth+20, plotHeight+30, 5, TFT_OUTLINE);
  
  // Prüfen, ob echte Effekte vorhanden sind
  bool alleNullWerte = true;
  for (int i = 0; i < 5; i++) {
    if (effekte[i] != 0) {
      alleNullWerte = false;
      break;
    }
  }
  
  // KORRIGIERT: Berechne Gesamtmittelwert und Response-Bereich für Y-Achsen-Skalierung
  float gesamtmittelwert = 0;
  float minResponse = 999;
  float maxResponse = -999;
  
  if (alleNullWerte) {
    // Testdaten
    gesamtmittelwert = 3.2;
    minResponse = 2.5;
    maxResponse = 4.0;
  } else {
    // Echte Daten
    for (int i = 0; i < 8; i++) {
      gesamtmittelwert += teilfaktoriellMittelwerte[i];
      if (teilfaktoriellMittelwerte[i] < minResponse) minResponse = teilfaktoriellMittelwerte[i];
      if (teilfaktoriellMittelwerte[i] > maxResponse) maxResponse = teilfaktoriellMittelwerte[i];
    }
    gesamtmittelwert /= 8.0;
    
    // Erweitere den Bereich um 10% für bessere Darstellung
    float range = maxResponse - minResponse;
    if (range < 0.5) range = 0.5; // Mindestbereich
    minResponse -= range * 0.1;
    maxResponse += range * 0.1;
  }
  
  // Faktoren in der oberen Reihe (erste 3)
  for (int i = 0; i < 3; i++) {
    int x = startX + i * factorWidth;
    int y = startY;
    
    // Rahmen mit abgerundeten Ecken
    tft.drawRoundRect(x, y, factorWidth, factorHeight, 3, TFT_GRID);
    
    // Farbigen Hintergrund für den Faktornamen
    tft.fillRoundRect(x+5, y+5, factorWidth-10, 20, 3, TFT_TITLE_BG);
    
    // Faktorname
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setCursor(x + factorWidth/2 - 20, y + 12);
    tft.print(faktorNamen[i]);
    
    // Y-Achse
    tft.drawFastVLine(x + 20, y + 25, factorHeight - 35, TFT_GRID);
    
    // X-Achse
    tft.drawFastHLine(x + 20, y + factorHeight - 15, factorWidth - 40, TFT_GRID);
    
    // X-Achsen-Positionen
    int x1 = x + 35;  // -1 Level
    int x2 = x + factorWidth - 35;  // +1 Level
    
    // Vertikale Linien für -1 und +1 Level
    tft.drawFastVLine(x1, y + factorHeight - 20, 10, TFT_GRID);
    tft.drawFastVLine(x2, y + factorHeight - 20, 10, TFT_GRID);
    
    // X-Achse Beschriftung
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x1 - 5, y + factorHeight - 10);
    tft.print("-1");
    tft.setCursor(x2 - 5, y + factorHeight - 10);
    tft.print("+1");
    
    // KORRIGIERT: Berechne echte Mittelwerte für -1 und +1 Level
    float mittelwert_niedrig = 0;
    float mittelwert_hoch = 0;
    int anzahl_niedrig = 0;
    int anzahl_hoch = 0;
    
    if (alleNullWerte) {
      // Testdaten - berechne aus Gesamtmittelwert und Effekt
      float testEffekte[5] = {0.5, -0.3, 0.2, -0.7, 0.4};
      mittelwert_niedrig = gesamtmittelwert - testEffekte[i] / 2.0;
      mittelwert_hoch = gesamtmittelwert + testEffekte[i] / 2.0;
    } else {
      // Echte Daten - berechne aus teilfaktoriellen Messungen
      for (int j = 0; j < 8; j++) {
        if (teilfaktoriellPlan[j][i] == -1) {
          mittelwert_niedrig += teilfaktoriellMittelwerte[j];
          anzahl_niedrig++;
        } else if (teilfaktoriellPlan[j][i] == 1) {
          mittelwert_hoch += teilfaktoriellMittelwerte[j];
          anzahl_hoch++;
        }
      }
      
      if (anzahl_niedrig > 0) mittelwert_niedrig /= anzahl_niedrig;
      if (anzahl_hoch > 0) mittelwert_hoch /= anzahl_hoch;
      
      // Fallback falls keine Daten vorhanden
      if (anzahl_niedrig == 0) {
        mittelwert_niedrig = gesamtmittelwert * 0.9;
      }
      if (anzahl_hoch == 0) {
        mittelwert_hoch = gesamtmittelwert * 1.1;
      }
    }
    
    // Y-Positionen basierend auf tatsächlichen Werten - VERBESSERTE Float-Berechnung
    float y_range = maxResponse - minResponse;
    // Sicherstellen, dass y_range nicht zu klein ist
    if (y_range < 0.5) y_range = 0.5;
    
    // Korrigierte Berechnung der Skalierung mit mehr Platz am Rand
    float y_scale = (float)(factorHeight - 60) / y_range;
    
    // Korrigierte Y-Positionen mit Begrenzung, damit sie im sichtbaren Bereich bleiben
    int y1 = y + factorHeight - 25 - (int)((mittelwert_niedrig - minResponse) * y_scale);
    int y2 = y + factorHeight - 25 - (int)((mittelwert_hoch - minResponse) * y_scale);
    
    // Sicherstellen, dass die Punkte innerhalb des Diagramms bleiben
    y1 = constrain(y1, y + 30, y + factorHeight - 20);
    y2 = constrain(y2, y + 30, y + factorHeight - 20);
    
    // Effekt-Linie zeichnen
    float effekt = mittelwert_hoch - mittelwert_niedrig;
    uint16_t lineColor = (effekt > 0) ? TFT_SUCCESS : TFT_WARNING;
    
    tft.drawLine(x1, y1, x2, y2, lineColor);
    // Dickere Linie für bessere Sichtbarkeit
    tft.drawLine(x1, y1-1, x2, y2-1, lineColor);
    tft.drawLine(x1, y1+1, x2, y2+1, lineColor);
    
    // Punkte an den Enden mit Farbverlauf
    for (int r = 4; r >= 0; r--) {
      uint16_t pointColor = r == 0 ? TFT_TEXT : lineColor;
      tft.fillCircle(x1, y1, r, pointColor);
      tft.fillCircle(x2, y2, r, pointColor);
    }
    
    // Y-Achsen-Beschriftung mit Werten
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x + 2, y1 - 3);
    tft.print(mittelwert_niedrig, 1);
    tft.setCursor(x + 2, y2 - 3);
    tft.print(mittelwert_hoch, 1);
    
    // Y-Achsen-Titel
    tft.setCursor(x + 2, y + 30);
    tft.print("uW");
  }
  
  // Faktoren in der unteren Reihe (letzte 2)
  for (int i = 0; i < 2; i++) {
    int faktorIndex = i + 3;
    int x = startX + i * factorWidth + factorWidth/2;
    int y = startY + factorHeight + 20;
    
    // Rahmen mit abgerundeten Ecken
    tft.drawRoundRect(x, y, factorWidth, factorHeight, 3, TFT_GRID);
    
    // Farbigen Hintergrund für den Faktornamen
    tft.fillRoundRect(x+5, y+5, factorWidth-10, 20, 3, TFT_TITLE_BG);
    
    // Faktorname
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setCursor(x + factorWidth/2 - 20, y + 12);
    tft.print(faktorNamen[faktorIndex]);
    
    // Y-Achse
    tft.drawFastVLine(x + 20, y + 25, factorHeight - 35, TFT_GRID);
    
    // X-Achse
    tft.drawFastHLine(x + 20, y + factorHeight - 15, factorWidth - 40, TFT_GRID);
    
    // X-Achsen-Positionen
    int x1 = x + 35;
    int x2 = x + factorWidth - 35;
    
    // Vertikale Linien für -1 und +1 Level
    tft.drawFastVLine(x1, y + factorHeight - 20, 10, TFT_GRID);
    tft.drawFastVLine(x2, y + factorHeight - 20, 10, TFT_GRID);
    
    // X-Achse Beschriftung
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x1 - 5, y + factorHeight - 10);
    tft.print("-1");
    tft.setCursor(x2 - 5, y + factorHeight - 10);
    tft.print("+1");
    
    // KORRIGIERT: Berechne echte Mittelwerte
    float mittelwert_niedrig = 0;
    float mittelwert_hoch = 0;
    int anzahl_niedrig = 0;
    int anzahl_hoch = 0;
    
    if (alleNullWerte) {
      float testEffekte[5] = {0.5, -0.3, 0.2, -0.7, 0.4};
      mittelwert_niedrig = gesamtmittelwert - testEffekte[faktorIndex] / 2.0;
      mittelwert_hoch = gesamtmittelwert + testEffekte[faktorIndex] / 2.0;
    } else {
      for (int j = 0; j < 8; j++) {
        if (teilfaktoriellPlan[j][faktorIndex] == -1) {
          mittelwert_niedrig += teilfaktoriellMittelwerte[j];
          anzahl_niedrig++;
        } else if (teilfaktoriellPlan[j][faktorIndex] == 1) {
          mittelwert_hoch += teilfaktoriellMittelwerte[j];
          anzahl_hoch++;
        }
      }
      
      if (anzahl_niedrig > 0) mittelwert_niedrig /= anzahl_niedrig;
      if (anzahl_hoch > 0) mittelwert_hoch /= anzahl_hoch;
      
      // Fallback falls keine Daten vorhanden
      if (anzahl_niedrig == 0) {
        mittelwert_niedrig = gesamtmittelwert * 0.9;
      }
      if (anzahl_hoch == 0) {
        mittelwert_hoch = gesamtmittelwert * 1.1;
      }
    }
    
    // Y-Positionen basierend auf tatsächlichen Werten - VERBESSERTE Float-Berechnung
    float y_range = maxResponse - minResponse;
    // Sicherstellen, dass y_range nicht zu klein ist
    if (y_range < 0.5) y_range = 0.5;
    
    // Korrigierte Berechnung der Skalierung mit mehr Platz am Rand
    float y_scale = (float)(factorHeight - 60) / y_range;
    
    // Korrigierte Y-Positionen mit Begrenzung, damit sie im sichtbaren Bereich bleiben
    int y1 = y + factorHeight - 25 - (int)((mittelwert_niedrig - minResponse) * y_scale);
    int y2 = y + factorHeight - 25 - (int)((mittelwert_hoch - minResponse) * y_scale);
    
    // Sicherstellen, dass die Punkte innerhalb des Diagramms bleiben
    y1 = constrain(y1, y + 30, y + factorHeight - 20);
    y2 = constrain(y2, y + 30, y + factorHeight - 20);
    
    // Effekt-Linie zeichnen
    float effekt = mittelwert_hoch - mittelwert_niedrig;
    uint16_t lineColor = (effekt > 0) ? TFT_SUCCESS : TFT_WARNING;
    
    tft.drawLine(x1, y1, x2, y2, lineColor);
    tft.drawLine(x1, y1-1, x2, y2-1, lineColor);
    tft.drawLine(x1, y1+1, x2, y2+1, lineColor);
    
    // Punkte an den Enden
    for (int r = 4; r >= 0; r--) {
      uint16_t pointColor = r == 0 ? TFT_TEXT : lineColor;
      tft.fillCircle(x1, y1, r, pointColor);
      tft.fillCircle(x2, y2, r, pointColor);
    }
    
    // Y-Werte anzeigen
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x + 2, y1 - 3);
    tft.print(mittelwert_niedrig, 1);
    tft.setCursor(x + 2, y2 - 3);
    tft.print(mittelwert_hoch, 1);
    
    tft.setCursor(x + 2, y + 30);
    tft.print("uW");
  }
  
  // Wenn Testdaten verwendet werden, Hinweis anzeigen
  if (alleNullWerte) {
    tft.fillRoundRect(startX+50, startY+2*factorHeight+30, 300, 20, 5, TFT_TITLE_BG);
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(startX+55, startY + 2*factorHeight + 35);
    tft.print("Beispieldaten bei -1/+1 Level");
  }
}

/**
 * KORRIGIERT: Zeigt ein Interaktions-Diagramm an
 * FEHLER BEHOBEN: Doppelte Deklaration von y_range entfernt
 */
void WindTurbineExperiment::zeigeInteraktionsDiagramm() {
  int startX = 60;
  int startY = 90;
  int plotWidth = 350;
  int plotHeight = 180;
  
  // Diagramm-Rahmen
  tft.drawRoundRect(startX-10, startY-30, plotWidth+20, plotHeight+40, 5, TFT_OUTLINE);
  
  // Titel
  tft.fillRoundRect(startX + plotWidth/2 - 80, startY - 25, 160, 20, 5, TFT_TITLE_BG);
  tft.setTextSize(1);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setCursor(startX + plotWidth/2 - 50, startY - 20);
  tft.println("Interaction Plot");
  
  // Rahmen zeichnen
  tft.drawRect(startX, startY, plotWidth, plotHeight, TFT_GRID);
  
  // Prüfen, ob echte Effekte vorhanden sind
  bool alleNullWerte = true;
  for (int i = 0; i < 5; i++) {
    if (effekte[i] != 0) {
      alleNullWerte = false;
      break;
    }
  }
  
  // Wähle die beiden stärksten Faktoren
  int faktor1 = 0;
  int faktor2 = 1;
  
  if (!alleNullWerte) {
    float maxEffekt1 = 0;
    float maxEffekt2 = 0;
    
    for (int i = 0; i < 5; i++) {
      float absEffekt = abs(effekte[i]);
      if (absEffekt > maxEffekt1) {
        maxEffekt2 = maxEffekt1;
        faktor2 = faktor1;
        maxEffekt1 = absEffekt;
        faktor1 = i;
      } else if (absEffekt > maxEffekt2) {
        maxEffekt2 = absEffekt;
        faktor2 = i;
      }
    }
  }
  
  // KORRIGIERT: Robuste Datensammlung mit Validierung
  float y_werte[4] = {0, 0, 0, 0}; // F1-/F2-, F1+/F2-, F1-/F2+, F1+/F2+
  bool datenVorhanden[4] = {false, false, false, false};
  int anzahlWerte[4] = {0, 0, 0, 0};
  
  if (alleNullWerte) {
    // Testdaten mit bekannter Interaktion
    y_werte[0] = 2.8; // F1-, F2-
    y_werte[1] = 3.9; // F1+, F2-
    y_werte[2] = 4.1; // F1-, F2+
    y_werte[3] = 4.2; // F1+, F2+
    for (int i = 0; i < 4; i++) datenVorhanden[i] = true;
  } else {
    // Echte Daten sammeln
    for (int i = 0; i < 8; i++) {
      int index = -1;
      
      if (teilfaktoriellPlan[i][faktor1] == -1 && teilfaktoriellPlan[i][faktor2] == -1) {
        index = 0;
      } else if (teilfaktoriellPlan[i][faktor1] == 1 && teilfaktoriellPlan[i][faktor2] == -1) {
        index = 1;
      } else if (teilfaktoriellPlan[i][faktor1] == -1 && teilfaktoriellPlan[i][faktor2] == 1) {
        index = 2;
      } else if (teilfaktoriellPlan[i][faktor1] == 1 && teilfaktoriellPlan[i][faktor2] == 1) {
        index = 3;
      }
      
      if (index >= 0) {
        y_werte[index] += teilfaktoriellMittelwerte[i];
        anzahlWerte[index]++;
        datenVorhanden[index] = true;
      }
    }
    
    // Mittelwerte berechnen
    for (int i = 0; i < 4; i++) {
      if (anzahlWerte[i] > 0) {
        y_werte[i] /= anzahlWerte[i];
      }
    }
    
    // KORRIGIERT: Interpolation für fehlende Punkte
    for (int i = 0; i < 4; i++) {
      if (!datenVorhanden[i]) {
        // Einfache Interpolation basierend auf Haupteffekten
        float gesamtmittelwert = 0;
        for (int j = 0; j < 8; j++) {
          gesamtmittelwert += teilfaktoriellMittelwerte[j];
        }
        gesamtmittelwert /= 8.0;
        
        // Schätze basierend auf Haupteffekten
        float effekt1_beitrag = (i & 1) ? effekte[faktor1] / 2 : -effekte[faktor1] / 2;
        float effekt2_beitrag = (i & 2) ? effekte[faktor2] / 2 : -effekte[faktor2] / 2;
        
        y_werte[i] = gesamtmittelwert + effekt1_beitrag + effekt2_beitrag;
      }
    }
  }
  
  // Y-Skala bestimmen mit verbesserter Berechnung
  float y_min = y_werte[0];
  float y_max = y_werte[0];
  for (int i = 1; i < 4; i++) {
    if (y_werte[i] < y_min) y_min = y_werte[i];
    if (y_werte[i] > y_max) y_max = y_werte[i];
  }
  
  // Sicherstellen, dass der Bereich ausreichend groß ist
  float y_range = y_max - y_min;
  if (y_range < 0.5) y_range = 0.5;
  
  // Mehr Platz am Rand für bessere Sichtbarkeit
  y_min -= y_range * 0.15;
  y_max += y_range * 0.15;
  
  // WICHTIG: y_range hier aktualisieren nach der Erweiterung
  y_range = y_max - y_min;
  
  // Achsen zeichnen
  tft.drawFastHLine(startX, startY + plotHeight - 30, plotWidth, TFT_TEXT);
  tft.drawFastVLine(startX, startY, plotHeight - 30, TFT_TEXT);
  
  // X-Achsen-Positionen
  int x1 = startX + plotWidth/4;
  int x2 = startX + 3*plotWidth/4;
  
  // X-Achsen-Ticks
  tft.drawFastVLine(x1, startY + plotHeight - 35, 10, TFT_TEXT);
  tft.drawFastVLine(x2, startY + plotHeight - 35, 10, TFT_TEXT);
  
  // X-Achsen-Beschriftung
  tft.fillRoundRect(startX + plotWidth/2 - 50, startY + plotHeight - 15, 100, 20, 5, TFT_TITLE_BG);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(startX + plotWidth/2 - 30, startY + plotHeight - 10);
  tft.print(faktorNamen[faktor1]);
  
  tft.fillRect(x1-10, startY + plotHeight - 25, 20, 15, 0x1082);
  tft.setTextColor(TFT_LIGHT_TEXT);
  tft.setCursor(x1-5, startY + plotHeight - 22);
  tft.print("-1");
  
  tft.fillRect(x2-10, startY + plotHeight - 25, 20, 15, 0x1082);
  tft.setCursor(x2-5, startY + plotHeight - 22);
  tft.print("+1");
  
  // Y-Achsen-Beschriftung
  tft.fillRect(startX - 25, startY + plotHeight/2 - 30, 20, 60, TFT_TITLE_BG);
  tft.setCursor(startX - 20, startY + plotHeight/2 + 10);
  tft.setTextColor(TFT_TEXT);
  tft.print("L");
  tft.setCursor(startX - 20, startY + plotHeight/2);
  tft.print("e");
  tft.setCursor(startX - 20, startY + plotHeight/2 - 10);
  tft.print("i");
  tft.setCursor(startX - 20, startY + plotHeight/2 - 20);
  tft.print("s");
  
  // Y-Skala
  tft.fillRect(startX - 30, startY + 5, 25, 20, 0x1082);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(startX - 28, startY + 10);
  tft.print(y_max, 1);
  
  tft.fillRect(startX - 30, startY + plotHeight - 40, 25, 20, 0x1082);
  tft.setCursor(startX - 28, startY + plotHeight - 35);
  tft.print(y_min, 1);
  
  // Legende
  tft.fillRoundRect(startX + plotWidth - 90, startY + 5, 80, 65, 5, TFT_OUTLINE);
  tft.fillRect(startX + plotWidth - 89, startY + 6, 78, 18, TFT_TITLE_BG);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setCursor(startX + plotWidth - 85, startY + 10);
  tft.print(faktorNamen[faktor2]);
  
  // Linie für Faktor2 = -1
  tft.setTextColor(TFT_TEXT);
  tft.drawFastHLine(startX + plotWidth - 80, startY + 35, 20, TFT_TEXT);
  for (int r = 3; r >= 0; r--) {
    tft.fillCircle(startX + plotWidth - 70, startY + 35, r, r == 0 ? TFT_TEXT : TFT_TEXT);
  }
  tft.setCursor(startX + plotWidth - 45, startY + 32);
  tft.print("-1");
  
  // Linie für Faktor2 = +1
  tft.drawFastHLine(startX + plotWidth - 80, startY + 55, 20, TFT_CHART_ACCENT);
  for (int r = 3; r >= 0; r--) {
    tft.fillCircle(startX + plotWidth - 70, startY + 55, r, r == 0 ? TFT_TEXT : TFT_CHART_ACCENT);
  }
  tft.setCursor(startX + plotWidth - 45, startY + 52);
  tft.print("+1");
  
  // KORRIGIERT: Linien und Punkte zeichnen (verwende bereits berechnete y_range)
  // Linie für Faktor 2 = -1 (Float-Berechnung mit verbesserter Skalierung)
  float y_scale = (float)(plotHeight - 50) / y_range;
  
  // Berechnung der Y-Positionen mit Begrenzung
  int y1_faktor2_minus = startY + plotHeight - 40 - (int)((y_werte[0] - y_min) * y_scale);
  int y2_faktor2_minus = startY + plotHeight - 40 - (int)((y_werte[1] - y_min) * y_scale);
  
  // Sicherstellen, dass die Punkte innerhalb des Diagramms bleiben
  y1_faktor2_minus = constrain(y1_faktor2_minus, startY + 10, startY + plotHeight - 40);
  y2_faktor2_minus = constrain(y2_faktor2_minus, startY + 10, startY + plotHeight - 40);
  
  // Hauptlinie
  tft.drawLine(x1, y1_faktor2_minus, x2, y2_faktor2_minus, TFT_TEXT);
  tft.drawLine(x1, y1_faktor2_minus-1, x2, y2_faktor2_minus-1, TFT_TEXT);
  tft.drawLine(x1, y1_faktor2_minus+1, x2, y2_faktor2_minus+1, TFT_TEXT);
  
  // Punkte
  for (int r = 4; r >= 0; r--) {
    uint16_t color = r == 0 ? TFT_TEXT : (r == 4 ? 0x3186 : TFT_TEXT);
    tft.fillCircle(x1, y1_faktor2_minus, r, color);
    tft.fillCircle(x2, y2_faktor2_minus, r, color);
  }
  
  // Kennzeichnung interpolierter Punkte
  if (!alleNullWerte) {
    if (!datenVorhanden[0]) {
      tft.drawCircle(x1, y1_faktor2_minus, 6, TFT_WARNING);
    }
    if (!datenVorhanden[1]) {
      tft.drawCircle(x2, y2_faktor2_minus, 6, TFT_WARNING);
    }
  }
  
  // Linie für Faktor 2 = +1 (Float-Berechnung mit verbesserter Skalierung)
  int y1_faktor2_plus = startY + plotHeight - 40 - (int)((y_werte[2] - y_min) * y_scale);
  int y2_faktor2_plus = startY + plotHeight - 40 - (int)((y_werte[3] - y_min) * y_scale);
  
  // Sicherstellen, dass die Punkte innerhalb des Diagramms bleiben
  y1_faktor2_plus = constrain(y1_faktor2_plus, startY + 10, startY + plotHeight - 40);
  y2_faktor2_plus = constrain(y2_faktor2_plus, startY + 10, startY + plotHeight - 40);
  
  tft.drawLine(x1, y1_faktor2_plus, x2, y2_faktor2_plus, TFT_CHART_ACCENT);
  tft.drawLine(x1, y1_faktor2_plus-1, x2, y2_faktor2_plus-1, TFT_CHART_ACCENT);
  tft.drawLine(x1, y1_faktor2_plus+1, x2, y2_faktor2_plus+1, TFT_CHART_ACCENT);
  
  for (int r = 4; r >= 0; r--) {
    uint16_t color = r == 0 ? TFT_TEXT : (r == 4 ? 0xFB86 : TFT_CHART_ACCENT);
    tft.fillCircle(x1, y1_faktor2_plus, r, color);
    tft.fillCircle(x2, y2_faktor2_plus, r, color);
  }
  
  if (!alleNullWerte) {
    if (!datenVorhanden[2]) {
      tft.drawCircle(x1, y1_faktor2_plus, 6, TFT_WARNING);
    }
    if (!datenVorhanden[3]) {
      tft.drawCircle(x2, y2_faktor2_plus, 6, TFT_WARNING);
    }
  }
  
  // Werte anzeigen
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(x1-15, y1_faktor2_minus-15);
  tft.print(y_werte[0], 1);
  tft.setCursor(x2-15, y2_faktor2_minus-15);
  tft.print(y_werte[1], 1);
  
  tft.setTextColor(TFT_CHART_ACCENT);
  tft.setCursor(x1-15, y1_faktor2_plus+10);
  tft.print(y_werte[2], 1);
  tft.setCursor(x2-15, y2_faktor2_plus+10);
  tft.print(y_werte[3], 1);
  
  // Warnung bei interpolierten Daten
  if (!alleNullWerte) {
    bool hatInterpolierte = false;
    for (int i = 0; i < 4; i++) {
      if (!datenVorhanden[i]) {
        hatInterpolierte = true;
        break;
      }
    }
    
    if (hatInterpolierte) {
      tft.fillRoundRect(startX, startY + plotHeight, 300, 20, 5, TFT_WARNING);
      tft.setTextColor(TFT_TEXT);
      tft.setCursor(startX+5, startY + plotHeight + 5);
      tft.print("⚠ Gepunktete Punkte = Interpoliert (unvollständige Daten)");
    }
  } else {
    tft.fillRoundRect(startX, startY + plotHeight, 250, 20, 5, TFT_TITLE_BG);
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(startX+5, startY + plotHeight + 5);
    tft.print("Beispieldaten für Interaktionsplot");
  }
}

/**
 * Zeigt ein Pareto-Diagramm mit konsistenter Skalierung an
 * MATHEMATISCH KORREKT: Balken und Kurve verwenden beide Prozent-Basis (0-100%)
 */
void WindTurbineExperiment::zeigeParetoEffekteDiagramm(int x, int y) {
  int diagrammHoehe = 140;
  int diagrammBreite = 220;
  
  // Hintergrund
  for (int i = 0; i < diagrammHoehe; i++) {
    uint16_t color = tft.color565(4, 10 + i/10, 20 + i/5);
    tft.drawFastHLine(x-diagrammBreite, y-diagrammHoehe+i, diagrammBreite, color);
  }
  
  // Rahmen
  tft.drawRoundRect(x-diagrammBreite, y-diagrammHoehe, diagrammBreite, diagrammHoehe, 5, TFT_OUTLINE);
  
  // Achsen
  tft.drawFastVLine(x-diagrammBreite, y-diagrammHoehe, diagrammHoehe, TFT_GRID);
  tft.drawFastHLine(x-diagrammBreite, y, diagrammBreite, TFT_GRID);
  tft.drawFastVLine(x, y-diagrammHoehe, diagrammHoehe, TFT_LIGHT_TEXT); // Rechte Y-Achse
  
  // Titel
  tft.fillRoundRect(x-diagrammBreite/2-60, y-diagrammHoehe-20, 120, 20, 5, TFT_TITLE_BG);
  tft.setTextColor(TFT_HEADER);
  tft.setTextSize(1);
  tft.setCursor(x-diagrammBreite/2-40, y-diagrammHoehe-15);
  tft.print("Pareto-Diagramm");
  
  // Prüfe Daten
  bool alleNullWerte = true;
  for (int i = 0; i < 5; i++) {
    if (effekte[i] != 0) {
      alleNullWerte = false;
      break;
    }
  }
  
  // Sortierte Effekte
  int sortierteFaktoren[5];
  float sortierteEffekte[5];
  
  for (int i = 0; i < 5; i++) {
    sortierteFaktoren[i] = i;
    sortierteEffekte[i] = alleNullWerte ? 
                        (i == 0 ? 0.9 : (i == 1 ? 0.8 : (i == 2 ? 0.6 : (i == 3 ? 0.4 : 0.3)))) : 
                        abs(effekte[i]);
  }
  
  // Sortieren
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4 - i; j++) {
      if (sortierteEffekte[j] < sortierteEffekte[j + 1]) {
        float tempEffekt = sortierteEffekte[j];
        sortierteEffekte[j] = sortierteEffekte[j + 1];
        sortierteEffekte[j + 1] = tempEffekt;
        
        int tempIndex = sortierteFaktoren[j];
        sortierteFaktoren[j] = sortierteFaktoren[j + 1];
        sortierteFaktoren[j + 1] = tempIndex;
      }
    }
  }
  
  // MATHEMATISCH KORREKT: Gesamtsumme für Prozentberechnung
  float summe = 0;
  for (int i = 0; i < 5; i++) {
    summe += sortierteEffekte[i];
  }
  
  // MATHEMATISCH KORREKT: Prozentanteile berechnen (beide Skalen verwenden Prozente)
  float prozentanteile[5];
  float kumulierterProzent[5];
  float laufendeSumme = 0;
  
  for (int i = 0; i < 5; i++) {
    prozentanteile[i] = (summe > 0) ? (sortierteEffekte[i] / summe) * 100.0 : 0;
    laufendeSumme += prozentanteile[i];
    kumulierterProzent[i] = laufendeSumme;
  }
  
  // Y-Achsen-Beschriftung (beide 0-100%)
  tft.fillRoundRect(x-diagrammBreite-50, y-diagrammHoehe/2-10, 45, 20, 3, TFT_TITLE_BG);
  tft.setTextColor(TFT_LIGHT_TEXT);
  tft.setCursor(x-diagrammBreite-45, y-diagrammHoehe/2-5);
  tft.print("Prozent");
  
  // Skala links (Einzeleffekte in %)
  for (int i = 0; i <= 4; i++) {
    int y_pos = y - (i * (diagrammHoehe-20)) / 4 - 10;
    tft.fillRoundRect(x-diagrammBreite-25, y_pos-8, 20, 16, 2, TFT_TITLE_BG);
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x-diagrammBreite-22, y_pos-3);
    tft.print(i * 25);
    tft.print("%");
  }
  
  // Skala rechts (Kumuliert 0-100%)
  for (int i = 0; i <= 4; i++) {
    int y_pos = y - (i * (diagrammHoehe-20)) / 4 - 10;
    tft.fillRoundRect(x+5, y_pos-8, 20, 16, 2, TFT_TITLE_BG);
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x+8, y_pos-3);
    tft.print(i * 25);
    tft.print("%");
  }
  
  // Referenzlinien
  for (int i = 1; i <= 4; i++) {
    int y_line = y - (i * (diagrammHoehe-20)) / 4 - 10;
    for (int x_dash = x-diagrammBreite+5; x_dash < x; x_dash += 10) {
      tft.drawFastHLine(x_dash, y_line, 5, 0x39E7);
    }
  }
  
  // Balkenbreite
  int balkenBreite = (diagrammBreite - 30) / 5;
  
  // MATHEMATISCH KORREKT: Balken und Kurve zeichnen (beide in Prozent)
  for (int i = 0; i < 5; i++) {
    int balkenX = x - diagrammBreite + 20 + i * balkenBreite;
    
    // MATHEMATISCH KORREKT: Balkenhöhe basierend auf Prozentanteil (nicht absoluter Wert)
    int balkenHoehe = (prozentanteile[i] / 100.0) * (diagrammHoehe - 20);
    if (balkenHoehe < 2 && prozentanteile[i] > 0) balkenHoehe = 2;
    
    // Balken zeichnen
    for (int h = 0; h < balkenHoehe; h++) {
      uint16_t currentColor = tft.color565(0, 100 - h/3, 150 + h/2);
      tft.drawFastHLine(balkenX, y - balkenHoehe - 10 + h, balkenBreite - 8, currentColor);
    }
    
    tft.drawRect(balkenX, y - balkenHoehe - 10, balkenBreite - 8, balkenHoehe, TFT_HIGHLIGHT);
    
    // Prozentanteil über dem Balken
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(balkenX, y - balkenHoehe - 25);
    tft.print(prozentanteile[i], 1);
    tft.print("%");
    
    // Faktorbezeichnung
    tft.fillRoundRect(balkenX, y + 5, balkenBreite - 8, 15, 3, TFT_TITLE_BG);
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(balkenX + (balkenBreite-8)/2 - 3, y + 8);
    tft.print(faktorNamen[sortierteFaktoren[i]][0]);
    
    // MATHEMATISCH KORREKT: Pareto-Kurve (kumulierte Prozente) mit verbesserter Positionierung
    int paretoY = y - (kumulierterProzent[i] / 100.0) * (diagrammHoehe - 20) - 10;
    // Sicherstellen, dass die Punkte innerhalb des Diagramms bleiben
    paretoY = constrain(paretoY, y - diagrammHoehe + 10, y - 10);
    
    int punktX = balkenX + (balkenBreite - 8) / 2;
    
    // Linie zur nächsten (falls vorhanden) mit verbesserter Positionierung
    if (i > 0) {
      int vorherX = x - diagrammBreite + 20 + (i-1) * balkenBreite + (balkenBreite - 8) / 2;
      int vorherY = y - (kumulierterProzent[i-1] / 100.0) * (diagrammHoehe - 20) - 10;
      
      // Sicherstellen, dass der vorherige Punkt innerhalb des Diagramms bleibt
      vorherY = constrain(vorherY, y - diagrammHoehe + 10, y - 10);
      
      tft.drawLine(vorherX, vorherY, punktX, paretoY, TFT_WARNING);
      tft.drawLine(vorherX, vorherY-1, punktX, paretoY-1, 0xFD40);
    }
    
    // Pareto-Punkt
    for (int r = 3; r >= 0; r--) {
      uint16_t color = r == 0 ? TFT_TEXT : (r == 3 ? 0xFD40 : TFT_WARNING);
      tft.fillCircle(punktX, paretoY, r, color);
    }
    
    // Kumulierter Prozentwert
    tft.setTextColor(TFT_WARNING);
    tft.setCursor(punktX + 8, paretoY - 5);
    tft.print((int)kumulierterProzent[i]);
    tft.print("%");
  }
  
  // Hinweis auf korrekte Skalierung
  if (alleNullWerte) {
    tft.fillRoundRect(x-diagrammBreite+20, y+25, 180, 15, 3, TFT_SUCCESS);
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(x-diagrammBreite+25, y+28);
    tft.print("Prozent-Skalierung für Balken und Kurve");
  }
}

/**
 * Zeigt ein Balkendiagramm der Effektstärken an
 * MATHEMATISCH KORREKT: Bereits korrekt implementiert
 */
void WindTurbineExperiment::zeigeEffekteDiagramm(int x, int y) {
  // Diagrammbereich mit abgerundeten Ecken
  int diagrammHoehe = 140;
  int diagrammBreite = 220;
  
  // Hintergrund mit leichtem Farbverlauf
  for (int i = 0; i < diagrammHoehe; i++) {
    uint16_t color = tft.color565(4, 10 + i/10, 20 + i/5);
    tft.drawFastHLine(x-diagrammBreite, y-diagrammHoehe+i, diagrammBreite, color);
  }
  
  // Rahmen zeichnen
  tft.drawRoundRect(x-diagrammBreite, y-diagrammHoehe, diagrammBreite, diagrammHoehe, 5, TFT_OUTLINE);
  
  // Y-Achse zeichnen
  tft.drawFastVLine(x-diagrammBreite, y-diagrammHoehe, diagrammHoehe, TFT_GRID);
  
  // X-Achse zeichnen
  tft.drawFastHLine(x-diagrammBreite, y, diagrammBreite, TFT_GRID);
  
  // Horizontale Nulllinie
  tft.drawFastHLine(x-diagrammBreite, y-diagrammHoehe/2, diagrammBreite, TFT_GRID);
  tft.fillRoundRect(x-diagrammBreite-25, y-diagrammHoehe/2-10, 20, 20, 3, TFT_TITLE_BG);
  tft.setTextColor(TFT_LIGHT_TEXT);
  tft.setTextSize(1);
  tft.setCursor(x-diagrammBreite-20, y-diagrammHoehe/2-5);
  tft.print("0");
  
  // Diagrammtitel
  tft.fillRoundRect(x-diagrammBreite/2-60, y-diagrammHoehe-20, 120, 20, 5, TFT_TITLE_BG);
  tft.setTextColor(TFT_HEADER);
  tft.setTextSize(1);
  tft.setCursor(x-diagrammBreite/2-40, y-diagrammHoehe-15);
  tft.print("Effektstaerken");
  
  // Finde maximalen Effekt
  float maxEffekt = 0;
  bool alleNullWerte = true;
  
  for (int i = 0; i < 5; i++) {
    if (abs(effekte[i]) > maxEffekt) {
      maxEffekt = abs(effekte[i]);
    }
    if (effekte[i] != 0) {
      alleNullWerte = false;
    }
  }
  
  if (alleNullWerte) {
    float testEffekte[5] = {0.5, -0.3, 0.2, -0.7, 0.4};
    maxEffekt = 0.7;
    
    int balkenBreite = (diagrammBreite - 30) / 5;
    float skalierung = (diagrammHoehe/2) / (maxEffekt * 1.2);
    
    for (int i = 0; i < 5; i++) {
      int balkenX = x - diagrammBreite + 20 + i * balkenBreite;
      int balkenHoehe = abs(testEffekte[i]) * skalierung;
      int balkenY = testEffekte[i] > 0 ? y - diagrammHoehe/2 - balkenHoehe : y - diagrammHoehe/2;
      
      if (testEffekte[i] > 0) {
        for (int h = 0; h < balkenHoehe; h++) {
          uint16_t color = tft.color565(0, 120 - h, 0);
          tft.drawFastHLine(balkenX, balkenY + h, balkenBreite-8, color);
        }
        tft.drawRect(balkenX, balkenY, balkenBreite-8, balkenHoehe, TFT_SUCCESS);
      } else {
        for (int h = 0; h < balkenHoehe; h++) {
          uint16_t color = tft.color565(120 - h, 0, 0);
          tft.drawFastHLine(balkenX, balkenY + h, balkenBreite-8, color);
        }
        tft.drawRect(balkenX, balkenY, balkenBreite-8, balkenHoehe, TFT_WARNING);
      }
      
      tft.setTextColor(TFT_TEXT);
      tft.setCursor(balkenX, testEffekte[i] > 0 ? balkenY - 15 : balkenY + balkenHoehe + 5);
      tft.print(testEffekte[i], 1);
    }
    
    tft.fillRoundRect(x-diagrammBreite+40, y-15, 140, 15, 3, TFT_TITLE_BG);
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x-diagrammBreite+45, y-12);
    tft.print("Beispieldaten");
  } else {
    // Reale Daten mit verbesserter Skalierung
    // Sicherstellen, dass die Skalierung nicht zu groß wird
    float skalierung = (diagrammHoehe/2 - 15) / (maxEffekt > 0 ? maxEffekt * 1.2 : 1);
    int balkenBreite = (diagrammBreite - 30) / 5;
    
    for (int i = 0; i < 5; i++) {
      int balkenX = x - diagrammBreite + 20 + i * balkenBreite;
      int balkenHoehe = abs(effekte[i]) * skalierung;
      if (balkenHoehe < 2 && effekte[i] != 0) balkenHoehe = 2;
      int balkenY = effekte[i] > 0 ? y - diagrammHoehe/2 - balkenHoehe : y - diagrammHoehe/2;
      
      if (effekte[i] > 0) {
        for (int h = 0; h < balkenHoehe; h++) {
          uint16_t color = tft.color565(0, 120 - h/2, 0);
          tft.drawFastHLine(balkenX, balkenY + h, balkenBreite-8, color);
        }
        tft.drawRect(balkenX, balkenY, balkenBreite-8, balkenHoehe, TFT_SUCCESS);
      } else if (effekte[i] < 0) {
        for (int h = 0; h < balkenHoehe; h++) {
          uint16_t color = tft.color565(120 - h/2, 0, 0);
          tft.drawFastHLine(balkenX, balkenY + h, balkenBreite-8, color);
        }
        tft.drawRect(balkenX, balkenY, balkenBreite-8, balkenHoehe, TFT_WARNING);
      }
      
      if (effekte[i] != 0) {
        tft.setTextColor(TFT_TEXT);
        tft.setCursor(balkenX, effekte[i] > 0 ? balkenY - 15 : balkenY + balkenHoehe + 5);
        tft.print(effekte[i], 1);
      }
    }
  }
  
  // Faktorbezeichnungen
  tft.setTextColor(TFT_TEXT);
  int balkenBreite = (diagrammBreite - 30) / 5;
  for (int i = 0; i < 5; i++) {
    int balkenX = x - diagrammBreite + 20 + i * balkenBreite;
    tft.fillRoundRect(balkenX, y+5, balkenBreite-8, 15, 3, TFT_TITLE_BG);
    tft.setCursor(balkenX + (balkenBreite-8)/2 - 3, y+8);
    tft.print(i+1);
  }
  
  // Legende
  tft.fillRoundRect(x-diagrammBreite+30, y+25, 160, 30, 5, TFT_OUTLINE);
  tft.setTextColor(TFT_LIGHT_TEXT);
  tft.setCursor(x-diagrammBreite+35, y+30);
  tft.print("1=St. 2=Gr. 3=Ab.");
  tft.setCursor(x-diagrammBreite+35, y+45);
  tft.print("4=Lu. 5=Bl.");
}

/**
 * Zeigt ein Balkendiagramm der vollfaktoriellen Versuchsergebnisse an
 * MATHEMATISCH KORREKT: Bereits korrekt implementiert
 */
void WindTurbineExperiment::zeigeVollfaktoriellDiagramm(int x, int y) {
  int diagrammHoehe = 140;
  int diagrammBreite = 220;
  
  // Hintergrund
  for (int i = 0; i < diagrammHoehe; i++) {
    uint16_t color = tft.color565(4, 10 + i/10, 20 + i/5);
    tft.drawFastHLine(x-diagrammBreite, y-diagrammHoehe+i, diagrammBreite, color);
  }
  
  tft.drawRoundRect(x-diagrammBreite, y-diagrammHoehe, diagrammBreite, diagrammHoehe, 5, TFT_OUTLINE);
  tft.drawFastVLine(x-diagrammBreite, y-diagrammHoehe, diagrammHoehe, TFT_GRID);
  tft.drawFastHLine(x-diagrammBreite, y, diagrammBreite, TFT_GRID);
  
  // Titel
  tft.fillRoundRect(x-diagrammBreite/2-70, y-diagrammHoehe-20, 140, 20, 5, TFT_TITLE_BG);
  tft.setTextColor(TFT_HEADER);
  tft.setTextSize(1);
  tft.setCursor(x-diagrammBreite/2-60, y-diagrammHoehe-15);
  tft.print("Vollfaktorieller Vergleich");
  
  bool alleNullWerte = true;
  for (int i = 0; i < 8; i++) {
    if (vollfaktoriellMittelwerte[i] != 0) {
      alleNullWerte = false;
      break;
    }
  }
  
  if (alleNullWerte) {
    float testWerte[8] = {0.8, 1.2, 0.7, 1.5, 0.9, 1.4, 1.1, 1.8};
    float minWert = 0.7;
    float maxWert = 1.8;
    
    float werteBereich = maxWert - minWert;
    float skalierung = (diagrammHoehe - 30) / werteBereich;
    int balkenBreite = (diagrammBreite - 20) / 8;
    
    for (int i = 0; i < 8; i++) {
      int balkenX = x - diagrammBreite + 10 + i * balkenBreite;
      int balkenHoehe = (testWerte[i] - minWert) * skalierung;
      if (balkenHoehe < 2) balkenHoehe = 2;
      
      for (int h = 0; h < balkenHoehe; h++) {
        uint16_t currentColor = tft.color565(0, 80 + h/3, 120 + h/2);
        tft.drawFastHLine(balkenX, y-h-15, balkenBreite-2, currentColor);
      }
      
      tft.drawRect(balkenX, y-balkenHoehe-15, balkenBreite-2, balkenHoehe, TFT_HIGHLIGHT);
      
      tft.setTextColor(TFT_TEXT);
      tft.setCursor(balkenX, y-balkenHoehe-30);
      tft.print(testWerte[i], 1);
    }
    
    tft.fillRoundRect(x-diagrammBreite+40, y-diagrammHoehe+5, 140, 15, 3, TFT_TITLE_BG);
    tft.setTextColor(TFT_LIGHT_TEXT);
    tft.setCursor(x-diagrammBreite+45, y-diagrammHoehe+8);
    tft.print("Beispieldaten");
  } else {
    // Reale Daten
    float minWert = vollfaktoriellMittelwerte[0];
    float maxWert = vollfaktoriellMittelwerte[0];
    
    for (int i = 1; i < 8; i++) {
      if (vollfaktoriellMittelwerte[i] < minWert) minWert = vollfaktoriellMittelwerte[i];
      if (vollfaktoriellMittelwerte[i] > maxWert) maxWert = vollfaktoriellMittelwerte[i];
    }
    
    if (minWert == maxWert) {
      if (minWert == 0) {
        maxWert = 1.0;
      } else {
        minWert = maxWert * 0.8;
        maxWert = maxWert * 1.2;
      }
    }
    
    float werteBereich = maxWert - minWert;
    // Verbesserte Skalierung mit mehr Platz am oberen Rand
    float skalierung = (diagrammHoehe - 40) / (werteBereich > 0 ? werteBereich : 1);
    int balkenBreite = (diagrammBreite - 20) / 8;
    
    for (int i = 0; i < 8; i++) {
      int balkenX = x - diagrammBreite + 10 + i * balkenBreite;
      int balkenHoehe = (vollfaktoriellMittelwerte[i] - minWert) * skalierung;
      if (balkenHoehe < 2 && vollfaktoriellMittelwerte[i] > 0) balkenHoehe = 2;
      
      // Sicherstellen, dass die Balken nicht zu hoch werden und im Diagramm bleiben
      balkenHoehe = constrain(balkenHoehe, 0, diagrammHoehe - 25);
      
      for (int h = 0; h < balkenHoehe; h++) {
        uint16_t currentColor = tft.color565(0, 80 + h/3, 120 + h/2);
        tft.drawFastHLine(balkenX, y-h-15, balkenBreite-2, currentColor);
      }
      
      tft.drawRect(balkenX, y-balkenHoehe-15, balkenBreite-2, balkenHoehe, TFT_HIGHLIGHT);
      
      tft.setTextColor(TFT_TEXT);
      tft.setCursor(balkenX, y-balkenHoehe-30);
      tft.print(vollfaktoriellMittelwerte[i], 1);
    }
  }
  
  // Versuchsnummern
  tft.setTextColor(TFT_TEXT);
  int balkenBreite = (diagrammBreite - 20) / 8;
  for (int i = 0; i < 8; i++) {
    int balkenX = x - diagrammBreite + 10 + i * balkenBreite;
    tft.fillRoundRect(balkenX, y+5, balkenBreite-2, 15, 3, TFT_TITLE_BG);
    tft.setCursor(balkenX + (balkenBreite-2)/2 - 3, y+8);
    tft.print(i+1);
  }
}
