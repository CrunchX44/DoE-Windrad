/**
 * WindTurbineCalculations.cpp
 * Berechnungsfunktionen für das Windkraftanlagen-Experiment
 */

 #include "WindTurbineExperiment.h"
 #include <Arduino.h>
 
/**
 * Berechnet den Mittelwert einer Messreihe
 * @param messungen Array mit Messwerten
 * @param anzahl Anzahl der Messwerte
 * @return Mittelwert der Messreihe
 */
 float WindTurbineExperiment::berechneMittelwert(float* messungen, int anzahl) {
   float summe = 0;
   for (int i = 0; i < anzahl; i++) {
     summe += messungen[i];
   }
   return summe / anzahl;
 }
 
/**
 * Berechnet die Standardabweichung einer Messreihe
 * @param messungen Array mit Messwerten
 * @param anzahl Anzahl der Messwerte
 * @param mittelwert Mittelwert der Messreihe
 * @return Standardabweichung der Messreihe
 */
 float WindTurbineExperiment::berechneStandardabweichung(float* messungen, int anzahl, float mittelwert) {
   float summeQuadraticheAbweichungen = 0;
   for (int i = 0; i < anzahl; i++) {
     float abweichung = messungen[i] - mittelwert;
     summeQuadraticheAbweichungen += abweichung * abweichung;
   }
   return sqrt(summeQuadraticheAbweichungen / (anzahl - 1));
 }
 
/**
 * Berechnet die Haupteffekte der Faktoren aus dem teilfaktoriellen Versuch
 * Zeigt eine Fortschrittsanzeige während der Berechnung
 */
 void WindTurbineExperiment::berechneEffekte() {
   // Statusanzeige für komplexe Berechnungen
   tft.fillScreen(TFT_BACKGROUND);
   zeichneTitelbalken("Effektberechnung");
   
   // Fortschrittsanzeige
   tft.fillRoundRect(20, 100, 440, 40, 5, TFT_OUTLINE);
   tft.setTextSize(1);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(180, 85);
   tft.print("Berechne Effekte...");
   
   // Bereich für aktuellen Faktor - feste Position
   tft.fillRoundRect(20, 150, 440, 30, 5, TFT_OUTLINE);
   
   // Für jeden Faktor
   for (int i = 0; i < 5; i++) {
     float summeNiedrig = 0;
     float summeHoch = 0;
     int anzahlNiedrig = 0;
     int anzahlHoch = 0;
     
     // Fortschrittsbalken aktualisieren
     int progress = (i * 440) / 5;
     tft.fillRect(21, 101, progress, 38, TFT_HIGHLIGHT);
     
     // Aktuellen Faktor anzeigen - Bereich vorher löschen
     tft.fillRect(21, 151, 438, 28, TFT_BACKGROUND);
     tft.setTextColor(TFT_HIGHLIGHT);
     tft.setCursor(30, 160);
     tft.print("Berechne Effekt fuer: ");
     tft.setTextColor(TFT_TEXT);
     tft.print(faktorNamen[i]);
     
     // Für jeden Versuch im teilfaktoriellen Plan
     for (int j = 0; j < 8; j++) {
       if (teilfaktoriellPlan[j][i] == -1) {
         summeNiedrig += teilfaktoriellMittelwerte[j];
         anzahlNiedrig++;
       } else if (teilfaktoriellPlan[j][i] == 1) {
         summeHoch += teilfaktoriellMittelwerte[j];
         anzahlHoch++;
       }
     }
     
     // Berechnung des Effekts
     float mittelwertNiedrig = summeNiedrig / anzahlNiedrig;
     float mittelwertHoch = summeHoch / anzahlHoch;
     effekte[i] = mittelwertHoch - mittelwertNiedrig;
     
     // Längere Pause für bessere Sichtbarkeit
     delay(800);
   }
   
   // Abschluss der Berechnung visuell darstellen
   tft.fillRect(21, 101, 438, 38, TFT_SUCCESS);
   
   // Aktuellen Faktor-Bereich für Abschlussmeldung nutzen
   tft.fillRect(21, 151, 438, 28, TFT_SUCCESS);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(180, 160);
   tft.print("Abgeschlossen!");
   
   delay(1000);
 }
 
/**
 * Bestimmt die drei wichtigsten Faktoren basierend auf den berechneten Effekten
 * Sortiert die Faktoren nach Effektstärke und wählt die drei stärksten für den vollfaktoriellen Versuch aus
 */
 void WindTurbineExperiment::bestimmeWichtigsteFaktoren() {
   // Statusanzeige für komplexe Berechnungen
   tft.fillScreen(TFT_BACKGROUND);
   zeichneTitelbalken("Faktorenanalyse");
   
   // Informationsbereich
   tft.fillRoundRect(20, 60, 440, 40, 5, TFT_OUTLINE);
   tft.setTextSize(1);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 75);
   tft.print("Bestimme die drei wichtigsten Faktoren...");
   
   // Kopie der Effekte und Indizes erstellen
   float absEffekte[5];
   int faktorIndizes[5];
   
   for (int i = 0; i < 5; i++) {
     absEffekte[i] = abs(effekte[i]);
     faktorIndizes[i] = i;
   }
   
   // Sortieren (Bubble-Sort) nach absoluten Effekten
   for (int i = 0; i < 4; i++) {
     for (int j = 0; j < 4 - i; j++) {
       if (absEffekte[j] < absEffekte[j + 1]) {
         // Effekte tauschen
         float tempEffekt = absEffekte[j];
         absEffekte[j] = absEffekte[j + 1];
         absEffekte[j + 1] = tempEffekt;
         
         // Indizes tauschen
         int tempIndex = faktorIndizes[j];
         faktorIndizes[j] = faktorIndizes[j + 1];
         faktorIndizes[j + 1] = tempIndex;
       }
     }
   }
   
   // Visualisierung der sortierten Faktoren
   tft.fillRoundRect(20, 110, 440, 160, 5, TFT_OUTLINE);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(30, 120);
   tft.println("Faktoren nach Effektstaerke sortiert:");
   
   // Faktoren mit Balken für relative Effektstärke anzeigen
   tft.setTextColor(TFT_TEXT);
   for (int i = 0; i < 5; i++) {
     int y = 145 + i * 25;
     
     // Zeilenhintergrund abwechselnd einfärben
     if (i % 2 == 0) {
       tft.fillRect(30, y-10, 420, 20, 0x1082);
     }
     
     // Rangplatz
     tft.setCursor(35, y);
     tft.print(i + 1);
     tft.print(". ");
     
     // Faktorname
     tft.setCursor(55, y);
     tft.print(faktorNamen[faktorIndizes[i]]);
     
     // Effektwert
     tft.setCursor(180, y);
     if (effekte[faktorIndizes[i]] >= 0) {
       tft.setTextColor(TFT_SUCCESS);
       tft.print("+");
     } else {
       tft.setTextColor(TFT_WARNING);
     }
     tft.print(effekte[faktorIndizes[i]], 2);
     tft.print(" uW");
     tft.setTextColor(TFT_TEXT);
     
     // Balken für relative Stärke
     int balkenBreite = (absEffekte[i] / absEffekte[0]) * 150;
     if (balkenBreite < 5 && absEffekte[i] > 0) balkenBreite = 5; // Mindestbreite
     
     if (effekte[faktorIndizes[i]] > 0) {
       tft.fillRect(280, y-7, balkenBreite, 14, TFT_SUCCESS);
     } else if (effekte[faktorIndizes[i]] < 0) {
       tft.fillRect(280, y-7, balkenBreite, 14, TFT_WARNING);
     }
     
     // Hervorhebung der ausgewählten Faktoren
     if (i < 3) {
       tft.drawRoundRect(28, y-12, 424, 24, 3, TFT_HIGHLIGHT);
     }
     
     // Animation verzögern
     delay(300);
   }
   
   // Die drei wichtigsten Faktoren auswählen
   for (int i = 0; i < 3; i++) {
     ausgewaehlteVollfaktoren[i] = faktorIndizes[i];
   }
   
   // NEUE FUNKTIONALITÄT: Fixierung der nicht ausgewählten Faktoren
   // Alle Faktoren zunächst als "nicht fixiert" markieren
   for (int i = 0; i < 5; i++) {
     fixierteFaktorwerte[i] = 99; // 99 = nicht fixiert
   }
   
   // Nicht ausgewählte Faktoren auf optimale Werte fixieren
   for (int i = 0; i < 5; i++) {
     bool istAusgewaehlt = false;
     
     // Prüfen ob dieser Faktor ausgewählt wurde
     for (int j = 0; j < 3; j++) {
       if (ausgewaehlteVollfaktoren[j] == i) {
         istAusgewaehlt = true;
         break;
       }
     }
     
     // Wenn nicht ausgewählt, auf optimale Stufe fixieren
     if (!istAusgewaehlt) {
       if (effekte[i] > 0.05) { // Schwellwert für positive Effekte
         fixierteFaktorwerte[i] = 1;  // Hohe Stufe (+) ist besser
       } else if (effekte[i] < -0.05) { // Schwellwert für negative Effekte
         fixierteFaktorwerte[i] = -1; // Niedrige Stufe (-) ist besser
       } else {
         fixierteFaktorwerte[i] = -1; // Bei geringem Effekt: niedrige Stufe (wirtschaftlicher)
       }
     }
   }
   
   // Auswahl bestätigen
   tft.fillRoundRect(120, 280, 240, 30, 5, TFT_SUCCESS);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(130, 290);
   tft.print("Ausgewaehlte Faktoren: ");
   
   for (int i = 0; i < 3; i++) {
     if (i > 0) tft.print(", ");
     tft.print(faktorNamen[ausgewaehlteVollfaktoren[i]][0]);
   }
   
   delay(1500);
   
  // Fixierung dem Benutzer anzeigen
  tft.fillScreen(TFT_BACKGROUND);
  zeichneTitelbalken("Faktoren-Fixierung");
  
  // Haupterklärung - kleinere Schrift
  tft.fillRoundRect(20, 60, 440, 30, 5, TFT_OUTLINE);
  tft.setTextSize(1); // Explizit kleinere Schrift setzen
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setCursor(30, 70);
  tft.print("Die nicht ausgewaehlten Faktoren werden auf optimale Werte fixiert:");
  
  // Variable Faktoren Sektion
  tft.fillRoundRect(20, 100, 440, 70, 5, TFT_OUTLINE);
  tft.fillRect(21, 101, 438, 18, TFT_SUCCESS);
  tft.setTextSize(1); // Kleinere Schrift für Header
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 107);
  tft.print("Variable Faktoren (werden in 8 Kombinationen variiert):");
  
  int yPos = 125;
  tft.setTextSize(1); // Kleinere Schrift für Listenelemente
  for (int i = 0; i < 3; i++) {
    int faktorIndex = ausgewaehlteVollfaktoren[i];
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(40, yPos);
    tft.print("- ");
    tft.print(faktorNamen[faktorIndex]);
    yPos += 12; // Kompakterer Zeilenabstand
  }
  
  // Fixierte Faktoren Sektion  
  tft.fillRoundRect(20, 180, 440, 80, 5, TFT_OUTLINE);
  tft.fillRect(21, 181, 438, 18, TFT_TITLE_BG);
  tft.setTextSize(1); // Kleinere Schrift für Header
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 187);
  tft.print("Fixierte Faktoren (bleiben konstant):");
  
  yPos = 205;
  tft.setTextSize(1); // Kleinere Schrift für Listenelemente
  for (int i = 0; i < 5; i++) {
    if (fixierteFaktorwerte[i] != 99) {
      tft.setTextColor(TFT_TEXT);
      tft.setCursor(40, yPos);
      tft.print("- ");
      tft.print(faktorNamen[i]);
      tft.print(": ");
      
      if (fixierteFaktorwerte[i] == 1) {
        tft.setTextColor(TFT_SUCCESS);
        tft.print(faktorEinheitenHoch[i]);
        tft.setTextColor(TFT_LIGHT_TEXT);
        tft.print(" (optimal)");
      } else {
        tft.setTextColor(TFT_LIGHT_TEXT);
        tft.print(faktorEinheitenNiedrig[i]);
        tft.print(" (optimal)");
      }
      tft.setTextColor(TFT_TEXT);
      yPos += 12; // Kompakterer Zeilenabstand
    }
  }
  
  // Erklärung - noch kleinere Schrift
  tft.fillRoundRect(20, 270, 440, 25, 5, TFT_SUBTITLE);
  tft.setTextSize(1); // Kleine Schrift für Fußnote
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 277);
  tft.print("Fixierung basiert auf Effektrichtung: positiv=hoch, negativ=niedrig");
  
  // Warten auf Benutzer-Eingabe
  zeichneStatusleiste("Druecken Sie den Drehknopf zum Fortfahren");
  
  bool warten = true;
  while (warten) {
    // Drehknopf prüfen
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
        buttonPressed = true;
        lastDebounceTime = millis();
        warten = false;
      }
    } else {
      buttonPressed = false;
    }
    
    // Keypad prüfen
    char key = keypad.getKey();
    if (key) {
      warten = false;
    }
    
    delay(50);
  }
}
 
/**
 * Berechnet einen Regressionskoeffizienten für das lineare Modell
 * @param koeffIndex Index des Koeffizienten (0=Konstante, 1-3=Faktoren)
 * @return Berechneter Regressionskoeffizient
 */
 float WindTurbineExperiment::berechneRegressionsKoeffizient(int koeffIndex) {
   // Erstellen des vollfaktoriellen Plans
   int vollfaktoriellPlan[8][5];
   for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 5; j++) {
       if (fixierteFaktorwerte[j] != 99) {
         // Fixierte Faktoren auf ihren festen Wert setzen
         vollfaktoriellPlan[i][j] = fixierteFaktorwerte[j];
       } else {
         vollfaktoriellPlan[i][j] = 0; // Wird gleich überschrieben
       }
     }
   }
   
   for (int i = 0; i < 8; i++) {
     vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[0]] = (i & 1) ? 1 : -1;
     vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[1]] = (i & 2) ? 1 : -1;
     vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[2]] = (i & 4) ? 1 : -1;
   }
   
   // Einfaches Beispiel für Regressionskoeffizienten
   // In einem realen Programm würde dies mit tatsächlichen Daten berechnet werden
   
   if (koeffIndex == 0) {
     // Konstanter Term (Mittelwert aller Versuchsergebnisse)
     float summe = 0;
     for (int i = 0; i < 8; i++) {
       summe += vollfaktoriellMittelwerte[i];
     }
     return summe / 8.0;
   } else {
     // Faktor-Koeffizienten
     int faktorIndex = ausgewaehlteVollfaktoren[koeffIndex - 1];
     
     float summeNiedrig = 0;
     float summeHoch = 0;
     int anzahlNiedrig = 0;
     int anzahlHoch = 0;
     
     for (int i = 0; i < 8; i++) {
       if (vollfaktoriellPlan[i][faktorIndex] == -1) {
         summeNiedrig += vollfaktoriellMittelwerte[i];
         anzahlNiedrig++;
       } else if (vollfaktoriellPlan[i][faktorIndex] == 1) {
         summeHoch += vollfaktoriellMittelwerte[i];
         anzahlHoch++;
       }
     }
     
     float mittelwertNiedrig = summeNiedrig / anzahlNiedrig;
     float mittelwertHoch = summeHoch / anzahlHoch;
     
     // Regressions-Koeffizient (Hälfte des Effekts)
     return (mittelwertHoch - mittelwertNiedrig) / 2.0;
   }
 }
 
/**
 * Berechnet das Bestimmtheitsmaß R² für das Regressionsmodell
 * @return R²-Wert zwischen 0 und 1
 */
 float WindTurbineExperiment::berechneR2() {
   // Regressionskoeffizienten berechnen
   float koeffizienten[4];
   for (int i = 0; i < 4; i++) {
     koeffizienten[i] = berechneRegressionsKoeffizient(i);
   }
   
   // Gesamtmittelwert berechnen
   float mittelwert = koeffizienten[0];
   
   // Summe der Quadrate der Abweichungen vom Mittelwert (SST)
   float sst = 0;
   for (int i = 0; i < 8; i++) {
     float abweichung = vollfaktoriellMittelwerte[i] - mittelwert;
     sst += abweichung * abweichung;
   }
   
   // Summe der Quadrate der Residuen (SSE)
   float sse = 0;
   for (int i = 0; i < 8; i++) {
     // Vorhersage für diesen Versuch berechnen
     float vorhersage = koeffizienten[0]; // Konstante
     
     for (int j = 0; j < 3; j++) {
       int faktorIndex = ausgewaehlteVollfaktoren[j];
       int faktorWert = (i & (1 << j)) ? 1 : -1;
       vorhersage += koeffizienten[j+1] * faktorWert;
     }
     
     // Residuum
     float residuum = vollfaktoriellMittelwerte[i] - vorhersage;
     sse += residuum * residuum;
   }
   
   // R² berechnen: 1 - (SSE / SST)
   float r2 = 1.0 - (sse / sst);
   
   // Begrenzung auf sinnvolle Werte (0 bis 1)
   if (r2 < 0) r2 = 0;
   if (r2 > 1) r2 = 1;
   
   return r2;
 }
 
/**
 * Berechnet die prognostizierte maximale Leistung bei optimalen Einstellungen
 * @return Prognostizierte maximale Leistung in µW
 */
float WindTurbineExperiment::berechnePrognose() {
  // Statusanzeige für komplexe Berechnungen
  tft.fillScreen(TFT_BACKGROUND);
  zeichneTitelbalken("Optimierungsberechnung");
  
  // Informationsbereich
  tft.fillRoundRect(20, 60, 440, 160, 5, TFT_OUTLINE);
  tft.setTextSize(1);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 70);
  tft.print("Berechne optimale Einstellungen...");
  
  // Animation für Prozessierung
  tft.fillRect(30, 90, 420, 20, TFT_BACKGROUND);
  for (int i = 0; i < 20; i++) {
    tft.fillRect(30 + i*21, 90, 20, 20, TFT_HIGHLIGHT);
    delay(50);
  }
  
  // Vorhersage für optimale Einstellungen
  // Konstanter Term
  float vorhersage = berechneRegressionsKoeffizient(0);
  
  // Optimale Einstellungen anzeigen
  tft.setTextColor(TFT_SUBTITLE);
  tft.setCursor(30, 120);
  tft.print("Optimale Faktorstufen:");
  
  // Für jeden ausgewählten Faktor
  for (int i = 0; i < 3; i++) {
    int faktorIndex = ausgewaehlteVollfaktoren[i];
    float koeffizient = berechneRegressionsKoeffizient(i + 1);
    int stufe = (koeffizient > 0) ? 1 : -1;
    
    // Je nach Vorzeichen des Koeffizienten die niedrige oder hohe Stufe wählen
    tft.setCursor(30, 140 + i*20);
    tft.print(faktorNamen[faktorIndex]);
    tft.print(": ");
    
    if (stufe > 0) {
      tft.setTextColor(TFT_HIGHLIGHT);
      tft.print(faktorEinheitenHoch[faktorIndex]);
      tft.print(" (+)");
      
      // Beitrag zur Vorhersage
      tft.setCursor(280, 140 + i*20);
      tft.print("+");
      tft.print(koeffizient, 2);
      tft.print(" uW");
      
      vorhersage += koeffizient * 1.0; // Hohe Stufe
    } else {
      tft.setTextColor(TFT_LIGHT_TEXT);
      tft.print(faktorEinheitenNiedrig[faktorIndex]);
      tft.print(" (-)");
      
      // Beitrag zur Vorhersage
      tft.setCursor(280, 140 + i*20);
      tft.print("-");
      tft.print(abs(koeffizient), 2);
      tft.print(" uW");
      
      vorhersage += koeffizient * (-1.0); // Niedrige Stufe
    }
    tft.setTextColor(TFT_SUBTITLE);
    
    // Berechnungsanimation
    tft.fillRect(200 + i*60, 200, 20, 10, TFT_SUCCESS);
    delay(300);
  }
  
  // Berechnetes Ergebnis anzeigen - NUR EINMAL!
  tft.fillRoundRect(20, 230, 440, 50, 5, TFT_SUCCESS);
  tft.setTextSize(1);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 230);
  tft.print("Prognostizierte maximale Leistung:");
  

  
  // Statusleiste mit korrektem Zurück-Button verwenden
  zeichneStatusleiste("Optimierung abgeschlossen - Druecken zum Fortfahren");
  
  delay(1000);
  
  return vorhersage;
}
