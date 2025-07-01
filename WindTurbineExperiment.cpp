/**
 * WindTurbineExperiment.cpp
 * Hauptimplementierung für das Windkraftanlagen-Experiment
 */

 #include "WindTurbineExperiment.h"

// Konstanten-Definition
const char* faktorNamen[] = {"Steigung", "Groesse", "Abstand", "Luftstaerke", "Blattanzahl"};
const char* faktorEinheitenNiedrig[] = {"4", "7 Zoll", "30 cm", "Stufe 1", "2 Stueck"};
const char* faktorEinheitenHoch[] = {"6", "8 Zoll", "60 cm", "Stufe 3", "3 Stueck"};
 
 // Teilfaktorieller Plan (2^(5-2) = 8 Versuche)
 const int teilfaktoriellPlan[8][5] = {
   {-1, -1, -1,  1,  1},  // Versuch 1: A-, B-, C-, D+, E+
   { 1, -1, -1, -1, -1},  // Versuch 2: A+, B-, C-, D-, E-
   {-1,  1, -1, -1,  1},  // Versuch 3: A-, B+, C-, D-, E+
   { 1,  1, -1,  1, -1},  // Versuch 4: A+, B+, C-, D+, E-
   {-1, -1,  1,  1, -1},  // Versuch 5: A-, B-, C+, D+, E-
   { 1, -1,  1, -1,  1},  // Versuch 6: A+, B-, C+, D-, E+
   {-1,  1,  1, -1, -1},  // Versuch 7: A-, B+, C+, D-, E-
   { 1,  1,  1,  1,  1}   // Versuch 8: A+, B+, C+, D+, E+
 };
 
 // Keypad-Layout
 char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
   {'1', '2', '3', 'A'},
   {'4', '5', '6', 'B'},
   {'7', '8', '9', 'C'},
   {'*', '0', '#', 'D'}
 };
 
byte rowPins[KEYPAD_ROWS] = {13, 14, 25, 26}; // Zeilen-Pins für das 4x4 Keypad
byte colPins[KEYPAD_COLS] = {5, 17, 27, 19};  // Spalten-Pins für das 4x4 Keypad
 
 // Konstruktor
WindTurbineExperiment::WindTurbineExperiment() : 
  keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS),
  tft(),
  encoder(),
  dataManager(),
  aktuellerModus(INTRO),
  vorherigerModus(INTRO),
  naechsterModus(INTRO),
  aktuellerVersuch(0),
  aktuelleMessung(0),
  encoderPosition(0),
  previousEncoderPosition(0),
  cursorPosition(0),
  maxCursorPosition(0),
  buttonPressed(false),
  lastDebounceTime(0),
  debounceDelay(250),
  anzahlGespeicherteVersuche(0),
  letzterMotorCheck(0),
  motorFehlerZaehler(0),
  motorWarnungAktiv(false),
  motorMonitoringPausiert(false),
  motorWarningPauseStart(0),
  motorStatusAktuell(true),
  letzteAkkuPruefung(0),        // NEU
  akkuSpannung(0),              // NEU
  akkuProzent(0)                // NEU
{
  // Initialisiere Standardwerte für ausgewählte Vollfaktoren
  ausgewaehlteVollfaktoren[0] = 0; // Steigung
  ausgewaehlteVollfaktoren[1] = 2; // Abstand
  ausgewaehlteVollfaktoren[2] = 4; // Blattanzahl
  
  // NEUE INITIALISIERUNG: Fixierte Faktorwerte
  for(int i = 0; i < 5; i++) {
    fixierteFaktorwerte[i] = 99; // 99 = nicht fixiert (Standardwert)
  }
  
  // Arrays mit 0 initialisieren
  for(int i = 0; i < 8; i++) {
    teilfaktoriellMittelwerte[i] = 0;
    teilfaktoriellStandardabweichungen[i] = 0;
    vollfaktoriellMittelwerte[i] = 0;
    vollfaktoriellStandardabweichungen[i] = 0;
    
    for(int j = 0; j < 5; j++) {
      teilfaktoriellMessungen[i][j] = 0;
      vollfaktoriellMessungen[i][j] = 0;
    }
  }
  
  for(int i = 0; i < 5; i++) {
    effekte[i] = 0;
  }
}
 
void WindTurbineExperiment::setup() {
  Serial.begin(115200);
  Serial.println("=== Windkraftanlagen-Experiment startet ===");
  
  // Display initialisieren mit den definierten Pins
  tft.init();
  tft.setRotation(1); // Landscape-Modus
  tft.fillScreen(TFT_BACKGROUND);
  tft.setTextColor(TFT_TEXT, TFT_BACKGROUND);
  
  // Hintergrundbeleuchtung einschalten (falls Pin definiert)
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  
  // I2C für INA226 konfigurieren
  Wire.begin(INA226_SDA, INA226_SCL);
  
  // INA226 initialisieren
  if (!ina226.begin()) {
    Serial.println("INA226 nicht gefunden!");
    tft.setTextSize(2);
    tft.setCursor(20, 40);
    tft.println("Fehler: INA226 nicht gefunden!");
    while (1) { delay(10); }
  }
  
  // Konfiguration für den INA226
  ina226.setMaxCurrentShunt(0.5, 0.1);
  ina226.setAverage(0);
  ina226.setBusVoltageConversionTime(0);
  ina226.setShuntVoltageConversionTime(0);
  
  // Encoder initialisieren
  encoder.attachHalfQuad(ENCODER_PIN_A, ENCODER_PIN_B);
  encoder.setCount(0);
  pinMode(ENCODER_BUTTON, INPUT_PULLUP);
  Serial.println("Drehregler initialisiert");
  
  // Dateisystem initialisieren
  if (!dataManager.begin()) {
    Serial.println("Fehler bei der Initialisierung des Dateisystems!");
    tft.setTextSize(2);
    tft.setCursor(20, 40);
    tft.println("Fehler: Dateisystem nicht initialisiert!");
    delay(3000);
  } else {
    Serial.println("Dateisystem erfolgreich initialisiert");
    Serial.println("AUTOMATISCHER RESET DEAKTIVIERT - Verwenden Sie die geheime Sequenz 9999 im Intro-Bildschirm");
  }

  // Motor-Test Pins konfigurieren
  pinMode(MOTOR_TEST_PIN_A, OUTPUT);
  pinMode(MOTOR_TEST_PIN_B, OUTPUT);
  digitalWrite(MOTOR_TEST_PIN_A, LOW);
  digitalWrite(MOTOR_TEST_PIN_B, LOW);
  
  Serial.println("Motor-Verbindungstest wird initialisiert...");
  
  // Startup Motor-Check durchführen
  startMotorStartupCheck();
  
  Serial.println("Setup abgeschlossen - zeige Intro");
  
  // Startbildschirm anzeigen
  zeigeIntro();
}
 
 void WindTurbineExperiment::loop() {

   // Akku-Monitoring (alle 30 Sekunden)
   handleAkkuMonitoring();
   // Motor-Background-Monitoring
   handleMotorBackgroundMonitoring();
   
   // Encoder-Position abfragen
   encoderPosition = encoder.getCount() / 2;
   
   // Prüfen auf Encoder-Drehung
   if (encoderPosition != previousEncoderPosition) {
     if (encoderPosition > previousEncoderPosition) {
       cursorPosition = min(cursorPosition + 1, maxCursorPosition);
     } else {
       cursorPosition = max(cursorPosition - 1, 0);
     }
     
     // UI aktualisieren basierend auf aktuellem Modus
     aktualisiereUI();
     previousEncoderPosition = encoderPosition;
   }
   
   // Prüfen auf Encoder-Button-Druck
   if (digitalRead(ENCODER_BUTTON) == LOW) {
     if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
       buttonPressed = true;
       lastDebounceTime = millis();
       verarbeiteButtonDruck();
     }
   } else {
     buttonPressed = false;
   }
   
   // Keypad abfragen
   char key = keypad.getKey();
   if (key) {
     verarbeiteKeypadEingabe(key);
   }
 }
 
 void WindTurbineExperiment::aktualisiereUI() {
   switch (aktuellerModus) {
     case INTRO:
       // Keine UI-Aktualisierung nötig
       break;
     case TEILFAKTORIELL_PLAN:
       // Keine UI-Aktualisierung nötig
       break;
     case TEILFAKTORIELL_MESSUNG:
       // Cursor-Position markieren
       break;
     case TEILFAKTORIELL_AUSWERTUNG:
       // Keine UI-Aktualisierung nötig
       break;
     case VOLLFAKTORIELL_PLAN:
       // Keine UI-Aktualisierung nötig
       break;
     case VOLLFAKTORIELL_MESSUNG:
       // Cursor-Position markieren
       break;
     case VOLLFAKTORIELL_AUSWERTUNG:
       // Keine UI-Aktualisierung nötig
       break;
     case REGRESSION:
       // Keine UI-Aktualisierung nötig
       break;
     case ZUSAMMENFASSUNG:
       // Keine UI-Aktualisierung nötig
       break;
     case BESTAETIGUNG_DIALOG:
       // Keine UI-Aktualisierung nötig
       break;
     case GESPEICHERTE_VERSUCHE:
       // Keine UI-Aktualisierung nötig
       break;
     case VERSUCH_DETAILS:
       // Keine UI-Aktualisierung nötig
       break;
     case WIFI_EXPORT:
       // Keine UI-Aktualisierung nötig
       break;
     case BESCHREIBUNG_EINGABE:
       // Keine UI-Aktualisierung nötig
       break;
   }
 }
 
void WindTurbineExperiment::verarbeiteButtonDruck() {
  // Vorherigen Modus speichern für Zurück-Funktion
  vorherigerModus = aktuellerModus;
  
  switch (aktuellerModus) {
    case INTRO:
      // Weiter zum teilfaktoriellen Plan
      zeigeTeilfaktoriellPlan();
      break;
    case TEILFAKTORIELL_PLAN:
      // Bestätigung vor Start der Messungen
      zeigeBestaetigung("Moechten Sie mit den Messungen beginnen?", TEILFAKTORIELL_MESSUNG);
      break;
    case BESTAETIGUNG_DIALOG:
      // Wird über Keypad-Eingabe verarbeitet
      break;
    case TEILFAKTORIELL_MESSUNG:
      // Messung durchführen
      if (aktuelleMessung < 5) {
        // Messung mit INA226 durchführen
        teilfaktoriellMessungen[aktuellerVersuch][aktuelleMessung] = messeLeistung();
        aktuelleMessung++;
        zeigeTeilfaktoriellMessung();
      } else {
        // Alle 5 Messungen abgeschlossen - Mittelwerte und Standardabweichungen berechnen
        teilfaktoriellMittelwerte[aktuellerVersuch] = berechneMittelwert(teilfaktoriellMessungen[aktuellerVersuch], 5);
        teilfaktoriellStandardabweichungen[aktuellerVersuch] = berechneStandardabweichung(teilfaktoriellMessungen[aktuellerVersuch], 5, teilfaktoriellMittelwerte[aktuellerVersuch]);

            // MOTOR-CHECK NACH JEDER 5. MESSUNG
        if (!testMotorVerbindung()) {
          motorStatusAktuell = false;
          
          // Motor-Warnung anzeigen
          tft.fillScreen(TFT_BACKGROUND);
          tft.fillRoundRect(50, 100, 380, 120, 8, TFT_WARNING);
          tft.setTextColor(TFT_TEXT);
          tft.setTextSize(1);
          tft.setCursor(70, 120);
          tft.println("Motor waehrend Messungen abgezogen!");
          tft.setCursor(70, 140);
          tft.print("Versuch ");
          tft.print(aktuellerVersuch + 1);
          tft.println(" muss wiederholt werden.");
          tft.setCursor(70, 160);
          tft.println("Bitte Motor anschliessen und # druecken");
          tft.setCursor(70, 180);
          tft.println("um diesen Versuch zu wiederholen.");
          
          // Warten auf Bestätigung
          bool warten = true;
          while (warten) {
            char key = keypad.getKey();
            if (key == '#') {
              // Prüfen ob Motor wieder da ist
              if (testMotorVerbindung()) {
                motorStatusAktuell = true;
                // Versuch komplett neu starten
                aktuelleMessung = 0;
                zeigeTeilfaktoriellMessung();
                warten = false;
              } else {
                // Immer noch nicht da
                tft.fillRect(70, 200, 300, 20, TFT_BACKGROUND);
                tft.setCursor(70, 200);
                tft.print("Motor immer noch nicht angeschlossen!");
                delay(2000);
                tft.fillRect(70, 200, 300, 20, TFT_BACKGROUND);
              }
            }
          }
          return; // Funktion verlassen
        }
        
        // Motor ist OK - Status aktualisieren
        motorStatusAktuell = true;
        
        // Bei bestimmten Versuchen manuelle Berechnungen vom Studenten fordern
        if (aktuellerVersuch == 1 || aktuellerVersuch == 4 || aktuellerVersuch == 6) {
          if (random(2) == 0) { // Zufällige Auswahl der Berechnungsart
            manuelleMittelwertEingabe(true, aktuellerVersuch);
          } else {
            manuelleStandardabweichungEingabe(true, aktuellerVersuch);
          }
          // Die manuelle Berechnung kümmert sich um den nächsten Versuch
        } else {
          // Nächster Versuch oder zur Auswertung
          aktuellerVersuch++;
          if (aktuellerVersuch < 8) {
            aktuelleMessung = 0;
            zeigeTeilfaktoriellMessung();
          } else {
            // Bestätigung vor Abschluss der teilfaktoriellen Versuche
            berechneEffekte();
            bestimmeWichtigsteFaktoren();
            // Manuelle Effekt-Berechnung für einen ausgewählten Faktor
            manuelleEffektBerechnung();
            // Bestätigung vor Anzeige der Auswertung
            zeigeBestaetigung("Alle Messungen abgeschlossen. Zur Auswertung?", TEILFAKTORIELL_AUSWERTUNG);
          }
        }
      }
      break;
    case TEILFAKTORIELL_AUSWERTUNG:
      // Bestätigung vor Wechsel zum vollfaktoriellen Versuch
      zeigeBestaetigung("Zum vollfaktoriellen Versuch wechseln?", VOLLFAKTORIELL_PLAN);
      break;
    case VOLLFAKTORIELL_PLAN:
      // Bestätigung vor Start der Messungen
      zeigeBestaetigung("Moechten Sie mit den Messungen beginnen?", VOLLFAKTORIELL_MESSUNG);
      break;
    case VOLLFAKTORIELL_MESSUNG:
      // Messung durchführen
      if (aktuelleMessung < 5) {
        // Messung mit INA226 durchführen
        vollfaktoriellMessungen[aktuellerVersuch][aktuelleMessung] = messeLeistung();
        aktuelleMessung++;
        zeigeVollfaktoriellMessung();
      } else {

        // Alle 5 Messungen abgeschlossen - Mittelwerte und Standardabweichungen berechnen
        vollfaktoriellMittelwerte[aktuellerVersuch] = berechneMittelwert(vollfaktoriellMessungen[aktuellerVersuch], 5);
        vollfaktoriellStandardabweichungen[aktuellerVersuch] = berechneStandardabweichung(vollfaktoriellMessungen[aktuellerVersuch], 5, vollfaktoriellMittelwerte[aktuellerVersuch]);
        // MOTOR-CHECK NACH JEDER 5. MESSUNG
        if (!testMotorVerbindung()) {
          motorStatusAktuell = false;
          
          // Motor-Warnung anzeigen
          tft.fillScreen(TFT_BACKGROUND);
          tft.fillRoundRect(50, 100, 380, 120, 8, TFT_WARNING);
          tft.setTextColor(TFT_TEXT);
          tft.setTextSize(1);
          tft.setCursor(70, 120);
          tft.println("Motor waehrend Messungen abgezogen!");
          tft.setCursor(70, 140);
          tft.print("Versuch ");
          tft.print(aktuellerVersuch + 1);
          tft.println(" muss wiederholt werden.");
          tft.setCursor(70, 160);
          tft.println("Bitte Motor anschliessen und # druecken");
          tft.setCursor(70, 180);
          tft.println("um diesen Versuch zu wiederholen.");
          
          // Warten auf Bestätigung
          bool warten = true;
          while (warten) {
            char key = keypad.getKey();
            if (key == '#') {
              // Prüfen ob Motor wieder da ist
              if (testMotorVerbindung()) {
                motorStatusAktuell = true;
                
                // NUR aktuellen Versuch wiederholen
                aktuelleMessung = 0;  // Messungen zurücksetzen
                // aktuellerVersuch bleibt gleich! ← WICHTIG
                
                // Bisherige Messungen löschen
                for (int i = 0; i < 5; i++) {
                  if (aktuellerModus == TEILFAKTORIELL_MESSUNG) {
                    teilfaktoriellMessungen[aktuellerVersuch][i] = 0;
                  } else {
                    vollfaktoriellMessungen[aktuellerVersuch][i] = 0;
                  }
                }
                
                // Zurück zur Messung
                if (aktuellerModus == TEILFAKTORIELL_MESSUNG) {
                  zeigeTeilfaktoriellMessung();
                } else {
                  zeigeVollfaktoriellMessung();
                }
                warten = false;
              } else {
                // Immer noch nicht da
                tft.fillRect(70, 200, 300, 20, TFT_BACKGROUND);
                tft.setCursor(70, 200);
                tft.print("Motor immer noch nicht angeschlossen!");
                delay(2000);
                tft.fillRect(70, 200, 300, 20, TFT_BACKGROUND);
              }
            }
          }
          return; // Funktion verlassen
        }
        
        // Motor ist OK - Status aktualisieren
        motorStatusAktuell = true;
        
        // Bei bestimmten Versuchen manuelle Berechnungen vom Studenten fordern
        if (aktuellerVersuch == 2 || aktuellerVersuch == 5) {
          if (random(2) == 0) { // Zufällige Auswahl der Berechnungsart
            manuelleMittelwertEingabe(false, aktuellerVersuch);
          } else {
            manuelleStandardabweichungEingabe(false, aktuellerVersuch);
          }
          // Die manuelle Berechnung kümmert sich um den nächsten Versuch
        } else {
          // Nächster Versuch oder zur Auswertung
          aktuellerVersuch++;
          if (aktuellerVersuch < 8) {
            aktuelleMessung = 0;
            zeigeVollfaktoriellMessung();
          } else {
            // Bestätigung vor Anzeige der Auswertung
            zeigeBestaetigung("Alle Messungen abgeschlossen. Zur Auswertung?", VOLLFAKTORIELL_AUSWERTUNG);
          }
        }
      }
      break;
    case VOLLFAKTORIELL_AUSWERTUNG:
      // Bestätigung vor Wechsel zum Regressionsmodell
      zeigeBestaetigung("Zum Regressionsmodell wechseln?", REGRESSION);
      break;
    case REGRESSION:
      // Bestätigung vor Wechsel zur Zusammenfassung
      zeigeBestaetigung("Zur Zusammenfassung wechseln?", ZUSAMMENFASSUNG);
      break;
    case ZUSAMMENFASSUNG:
      // Direkt zum Startbildschirm zurückkehren
      // Zuerst die Liste der gespeicherten Versuche aktualisieren
      anzahlGespeicherteVersuche = dataManager.listExperiments(gespeicherteVersuche, MAX_SAVED_EXPERIMENTS);
      
      // Dann zum Startbildschirm zurückkehren
      zeigeIntro();
      break;
    case GESPEICHERTE_VERSUCHE:
      // Wird in der Funktion selbst behandelt
      zeigeGespeicherteVersuche();
      break;
    case VERSUCH_DETAILS:
      // Wird in der Funktion selbst behandelt
      zeigeVersuchDetails(aktuellerVersuchsFilename);
      break;
    case WIFI_EXPORT:
      // Wird in der Funktion selbst behandelt
      zeigeWiFiExport();
      break;
  }
}
 
 // Bestätigungsdialog anzeigen
 void WindTurbineExperiment::zeigeBestaetigung(const char* nachricht, ProgrammModus zielModus) {
   // Vorherigen Modus speichern
   vorherigerModus = aktuellerModus;
   // Zielmodus speichern
   naechsterModus = zielModus;
   // Modus auf Bestätigungsdialog setzen
   aktuellerModus = BESTAETIGUNG_DIALOG;
   
   // Hintergrund abdunkeln
   tft.fillScreen(TFT_BACKGROUND);
   
   // Dialog-Box zentriert
   tft.fillRoundRect(90, 120, 300, 100, 8, TFT_OUTLINE);
   tft.fillRoundRect(92, 122, 296, 96, 6, TFT_BACKGROUND);
   
   // Titel
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setTextSize(1);
   tft.setCursor(110, 135);
   tft.print("Bestätigung erforderlich");
   
   // Nachricht
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(110, 155);
   tft.print(nachricht);
   
   // Ja/Nein Buttons
   tft.fillRoundRect(120, 180, 100, 30, 5, TFT_SUCCESS);
   tft.fillRoundRect(260, 180, 100, 30, 5, TFT_WARNING);
   
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(155, 190);
   tft.print("Ja (#)");
   tft.setCursor(290, 190);
   tft.print("Nein (*)");
   
   // Anleitung
   zeichneStatusleiste("# = Bestaetigen, * = Abbrechen, D = Zurueck");
 }
 
 // Zurück zum vorherigen Modus
 void WindTurbineExperiment::zurueckZumVorherigenModus() {
   // Wenn wir uns im Bestätigungsdialog befinden, gehen wir zum Modus zurück,
   // von dem aus der Dialog aufgerufen wurde
   if (aktuellerModus == BESTAETIGUNG_DIALOG) {
     aktuellerModus = vorherigerModus;
   } else if (aktuellerModus == ZUSAMMENFASSUNG) {
     // Von der Zusammenfassung direkt zum Startbildschirm zurückkehren
     aktuellerModus = INTRO;
   } else {
     // Sonst zum vorherigen Modus zurückkehren
     ProgrammModus tempModus = aktuellerModus;
     aktuellerModus = vorherigerModus;
     vorherigerModus = tempModus;
   }
   
   // UI entsprechend aktualisieren
   switch (aktuellerModus) {
     case INTRO:
       zeigeIntro();
       break;
     case TEILFAKTORIELL_PLAN:
       zeigeTeilfaktoriellPlan();
       break;
     case TEILFAKTORIELL_MESSUNG:
       zeigeTeilfaktoriellMessung();
       break;
     case TEILFAKTORIELL_AUSWERTUNG:
       zeigeTeilfaktoriellAuswertung();
       break;
     case VOLLFAKTORIELL_PLAN:
       zeigeVollfaktoriellPlan();
       break;
     case VOLLFAKTORIELL_MESSUNG:
       zeigeVollfaktoriellMessung();
       break;
     case VOLLFAKTORIELL_AUSWERTUNG:
       zeigeVollfaktoriellAuswertung();
       break;
     case REGRESSION:
       zeigeRegressionModell();
       break;
     case ZUSAMMENFASSUNG:
       zeigeZusammenfassung();
       break;
   }
 }
 
void WindTurbineExperiment::verarbeiteKeypadEingabe(char key) {
  // Motor-Warnung behandeln (hat Priorität vor allem anderen)
  if (motorWarnungAktiv) {
    if (key == '*') {
      // Monitoring für 60 Sekunden pausieren
      motorMonitoringPausiert = true;
      motorWarningPauseStart = millis();
      versteckeMotorWarnung();
      Serial.println("Motor-Monitoring für 60 Sekunden pausiert");
      return;
    } else if (key == '#') {
      // Sofortiger Neutest
      motorFehlerZaehler = 0; // Reset Fehler-Counter
      if (testMotorVerbindung()) {
        versteckeMotorWarnung();
        Serial.println("Motor-Test erfolgreich - Warnung entfernt");
      } else {
        motorFehlerZaehler = 5; // Warnung bleibt
        Serial.println("Motor immer noch nicht angeschlossen");
      }
      return;
    }
    // Andere Tasten werden ignoriert während Warnung aktiv ist
    return;
  }

  // Globale Tasten für alle Modi
  if (key == 'D') {
    // Zurück-Taste
    
    // Spezielle Behandlung für Messungsbildschirme
    if (aktuellerModus == TEILFAKTORIELL_MESSUNG) {
      if (aktuellerVersuch > 0) {
        // Zum vorherigen Versuch zurückgehen
        aktuellerVersuch--;
        
        // Anzahl der vorhandenen Messungen für diesen Versuch bestimmen
        int vorhandeneMessungen = 0;
        for (int i = 0; i < 5; i++) {
          if (teilfaktoriellMessungen[aktuellerVersuch][i] != 0) {
            vorhandeneMessungen++;
          } else {
            break;
          }
        }
        
        // Wenn alle 5 Messungen vorhanden sind, dann auf Messung 5 setzen
        // damit der Benutzer fortfahren kann
        aktuelleMessung = (vorhandeneMessungen >= 5) ? 5 : vorhandeneMessungen;
        
        // Debug-Ausgabe
        Serial.print("Zurück zu Versuch ");
        Serial.print(aktuellerVersuch);
        Serial.print(" mit ");
        Serial.print(aktuelleMessung);
        Serial.println(" Messungen");
        
        zeigeTeilfaktoriellMessung();
      } else {
        // Zum Plan zurückgehen
        zeigeTeilfaktoriellPlan();
      }
      return;
    } else if (aktuellerModus == VOLLFAKTORIELL_MESSUNG) {
      if (aktuellerVersuch > 0) {
        // Zum vorherigen Versuch zurückgehen
        aktuellerVersuch--;
        
        // Anzahl der vorhandenen Messungen für diesen Versuch bestimmen
        int vorhandeneMessungen = 0;
        for (int i = 0; i < 5; i++) {
          if (vollfaktoriellMessungen[aktuellerVersuch][i] != 0) {
            vorhandeneMessungen++;
          } else {
            break;
          }
        }
        
        // Wenn alle 5 Messungen vorhanden sind, dann auf Messung 5 setzen
        // damit der Benutzer fortfahren kann
        aktuelleMessung = (vorhandeneMessungen >= 5) ? 5 : vorhandeneMessungen;
        
        // Debug-Ausgabe
        Serial.print("Zurück zu Versuch ");
        Serial.print(aktuellerVersuch);
        Serial.print(" mit ");
        Serial.print(aktuelleMessung);
        Serial.println(" Messungen");
        
        zeigeVollfaktoriellMessung();
      } else {
        // Zum Plan zurückgehen
        zeigeVollfaktoriellPlan();
      }
      return;
    }
    
    // Standard-Zurückfunktion für alle anderen Modi
    zurueckZumVorherigenModus();
    return;
  }
  
  // Bestätigungsdialog-Modus
  if (aktuellerModus == BESTAETIGUNG_DIALOG) {
    if (key == '#') {
      // Bestätigen - zum nächsten Modus wechseln
      aktuellerModus = naechsterModus;
      
      // UI entsprechend aktualisieren
      switch (aktuellerModus) {
        case INTRO:
          zeigeIntro();
          break;
        case TEILFAKTORIELL_PLAN:
          zeigeTeilfaktoriellPlan();
          break;
        case TEILFAKTORIELL_MESSUNG:
          aktuellerVersuch = 0;
          aktuelleMessung = 0;
          zeigeTeilfaktoriellMessung();
          break;
        case TEILFAKTORIELL_AUSWERTUNG:
          zeigeTeilfaktoriellAuswertung();
          break;
        case VOLLFAKTORIELL_PLAN:
          aktuellerVersuch = 0;
          aktuelleMessung = 0;
          zeigeVollfaktoriellPlan();
          break;
        case VOLLFAKTORIELL_MESSUNG:
          aktuellerVersuch = 0;
          aktuelleMessung = 0;
          zeigeVollfaktoriellMessung();
          break;
        case VOLLFAKTORIELL_AUSWERTUNG:
          zeigeVollfaktoriellAuswertung();
          break;
        case REGRESSION:
          zeigeRegressionModell();
          break;
        case ZUSAMMENFASSUNG:
          zeigeZusammenfassung();
          break;
        case BESCHREIBUNG_EINGABE:
          zeigeBeschreibungEingabe();
          break;
        case GESPEICHERTE_VERSUCHE:
          zeigeGespeicherteVersuche();
          break;
        case VERSUCH_DETAILS:
          zeigeVersuchDetails(aktuellerVersuchsFilename);
          break;
        case WIFI_EXPORT:
          zeigeWiFiExport();
          break;
      }
    } else if (key == '*') {
      // Abbrechen - zum vorherigen Modus zurückkehren
      zurueckZumVorherigenModus();
    }
    return;
  }
  
  // Modus-spezifische Tasten
  if (aktuellerModus == INTRO) {
    // NEUE FUNKTION: Geheime Reset-Sequenz
    static String resetSequenz = "";
    static unsigned long letzteEingabe = 0;
    
    // Reset der Sequenz nach 5 Sekunden Inaktivität
    if (millis() - letzteEingabe > 5000) {
      resetSequenz = "";
    }
    
    // Reset-Sequenz-Verarbeitung
    if (key >= '0' && key <= '9') {
      resetSequenz += key;
      letzteEingabe = millis();
      
      // Debug-Ausgabe (nur für Entwicklung - kann entfernt werden)
      Serial.print("Reset-Sequenz Progress: ");
      for (int i = 0; i < resetSequenz.length(); i++) {
        Serial.print("*"); // Versteckte Anzeige für Sicherheit
      }
      Serial.println();
      
      // Geheime Sequenz: 9999 für manuellen Reset
      if (resetSequenz == "9999") {
        Serial.println("🔥 Geheime Reset-Sequenz erkannt!");
        resetSequenz = "";
        manuelleDatenLoeschung();
        return;
      }
      
      // Sequenz zurücksetzen wenn zu lang
      if (resetSequenz.length() > 4) {
        resetSequenz = "";
      }
      
      // Wenn es sich um normale Menü-Optionen handelt, weiter verarbeiten
      if (resetSequenz.length() == 1) {
        // Nur bei einstelligen Eingaben normale Menü-Logik ausführen
        if (key == '1') {
          // Gespeicherte Versuche anzeigen
          resetSequenz = ""; // Reset-Sequenz zurücksetzen
          anzahlGespeicherteVersuche = dataManager.listExperiments(gespeicherteVersuche, MAX_SAVED_EXPERIMENTS);
          zeigeGespeicherteVersuche();
        } else if (key == '2') {
          // Daten exportieren - wenn Versuche vorhanden sind
          resetSequenz = ""; // Reset-Sequenz zurücksetzen
          anzahlGespeicherteVersuche = dataManager.listExperiments(gespeicherteVersuche, MAX_SAVED_EXPERIMENTS);
          if (anzahlGespeicherteVersuche > 0) {
            // Zur Liste der gespeicherten Versuche wechseln, damit der Benutzer auswählen kann
            vorherigerModus = INTRO; // Speichern, dass wir vom Intro-Bildschirm kommen
            zeigeGespeicherteVersuche();
          } else {
            // Fehlermeldung anzeigen, wenn keine Versuche vorhanden sind
            tft.fillRoundRect(90, 120, 300, 80, 8, TFT_WARNING);
            tft.setTextColor(TFT_TEXT);
            tft.setTextSize(1);
            tft.setCursor(110, 140);
            tft.println("Keine gespeicherten Versuche vorhanden!");
            tft.setCursor(110, 160);
            tft.println("Drücken Sie eine Taste, um fortzufahren...");
            
            // Warten auf Tastendruck
            bool warten = true;
            while (warten) {
              char k = keypad.getKey();
              if (k) {
                warten = false;
              }
              
              // Auch auf Encoder-Button prüfen
              if (digitalRead(ENCODER_BUTTON) == LOW) {
                if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
                  buttonPressed = true;
                  lastDebounceTime = millis();
                  warten = false;
                }
              } else {
                buttonPressed = false;
              }
            }
            
            // Zurück zum Startbildschirm
            zeigeIntro();
          }
        }
      }
    }
  } else if (aktuellerModus == TEILFAKTORIELL_MESSUNG) {
    if (key == '*' && aktuelleMessung > 0) {
      // Letzte Messung löschen
      aktuelleMessung--;
      // Messwert auf 0 setzen
      teilfaktoriellMessungen[aktuellerVersuch][aktuelleMessung] = 0;
      zeigeTeilfaktoriellMessung();
    }
  } else if (aktuellerModus == VOLLFAKTORIELL_MESSUNG) {
    if (key == '*' && aktuelleMessung > 0) {
      // Letzte Messung löschen
      aktuelleMessung--;
      // Messwert auf 0 setzen
      vollfaktoriellMessungen[aktuellerVersuch][aktuelleMessung] = 0;
      zeigeVollfaktoriellMessung();
    }
  } else if (aktuellerModus == TEILFAKTORIELL_AUSWERTUNG) {
    if (key == '*') {
      // Manuelle Eingabe von Berechnungsergebnissen - zurück zur Auswertung
      manuelleStandardabweichungEingabe(true, 0, true); // Versuchsindex 0 als Platzhalter für Auswertung
    } else if (key == '#') {
      // Manuelle Eingabe von Mittelwerten - zurück zur Auswertung
      manuelleMittelwertEingabe(true, 0, true); // Versuchsindex 0 als Platzhalter für Auswertung
    } else if (key == '9') {
      // Manuelle Berechnung der Effekte
      manuelleEffektBerechnung();
    } else if (key == '1') {
      // Effekte-Diagramm anzeigen
      zeigeTeilfaktoriellDiagrammAnsicht();
    } else if (key == '2') {
      // Main Effects Plot anzeigen
      zeigeHaupteffekteDiagrammAnsicht();
    } else if (key == '3') {
      // Interaction Plot anzeigen
      zeigeInteraktionsDiagrammAnsicht();
    } else if (key == '4') {
      // Pareto-Diagramm anzeigen
      zeigeParetoEffekteDiagrammAnsicht();
    }
  } else if (aktuellerModus == VOLLFAKTORIELL_AUSWERTUNG) {
    if (key == '*') {
      // Manuelle Eingabe von Berechnungsergebnissen - zurück zur Auswertung
      manuelleStandardabweichungEingabe(false, 0, true); // Versuchsindex 0 als Platzhalter für Auswertung
    } else if (key == '#') {
      // Manuelle Eingabe von Mittelwerten - zurück zur Auswertung
      manuelleMittelwertEingabe(false, 0, true); // Versuchsindex 0 als Platzhalter für Auswertung
    } else if (key == '1') {
      // Diagramm anzeigen
      zeigeVollfaktoriellDiagrammAnsicht();
    }
  } else if (aktuellerModus == ZUSAMMENFASSUNG) {
    if (key == '1') {
      // Gespeicherte Versuche anzeigen
      zeigeGespeicherteVersuche();
    }
  } else if (aktuellerModus == GESPEICHERTE_VERSUCHE) {
    // Verarbeitung für gespeicherte Versuche
    if (key >= '1' && key <= '9') {
      int index = key - '1';
      if (index < anzahlGespeicherteVersuche) {
        // Versuch auswählen
        strcpy(aktuellerVersuchsFilename, gespeicherteVersuche[index].filename);
        zeigeVersuchDetails(aktuellerVersuchsFilename);
      }
    }
  } else if (aktuellerModus == VERSUCH_DETAILS) {
    // Verarbeitung für Versuchsdetails
    if (key == '1') {
      // WiFi-Export starten
      zeigeWiFiExport();
    } else if (key == '2') {
      // Versuch löschen
      
      // Bestätigung anfordern
      tft.fillScreen(TFT_BACKGROUND);
      tft.fillRoundRect(90, 120, 300, 80, 8, TFT_WARNING);
      tft.setTextColor(TFT_TEXT);
      tft.setTextSize(1);
      tft.setCursor(110, 140);
      tft.println("Versuch wirklich löschen?");
      tft.setCursor(110, 160);
      tft.println("# = Ja, * = Nein");
      
      // Auf Bestätigung warten
      bool warten = true;
      while (warten) {
        char k = keypad.getKey();
        if (k == '#') {
          // Löschen bestätigt
          warten = false;
          
          if (dataManager.deleteExperiment(aktuellerVersuchsFilename)) {
            // Erfolgsmeldung anzeigen
            tft.fillScreen(TFT_BACKGROUND);
            tft.fillRoundRect(90, 120, 300, 80, 8, TFT_SUCCESS);
            tft.setTextColor(TFT_TEXT);
            tft.setTextSize(1);
            tft.setCursor(110, 140);
            tft.println("Versuch erfolgreich gelöscht!");
            tft.setCursor(110, 160);
            tft.println("Drücken Sie eine Taste, um fortzufahren...");
            
            // Warten auf Tastendruck
            bool warten2 = true;
            while (warten2) {
              char k2 = keypad.getKey();
              if (k2) {
                warten2 = false;
              }
              
              // Auch auf Encoder-Button prüfen
              if (digitalRead(ENCODER_BUTTON) == LOW) {
                if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
                  buttonPressed = true;
                  lastDebounceTime = millis();
                  warten2 = false;
                }
              } else {
                buttonPressed = false;
              }
            }
            
            // Zurück zur Liste der gespeicherten Versuche
            zeigeGespeicherteVersuche();
          } else {
            // Fehlermeldung anzeigen
            tft.fillScreen(TFT_BACKGROUND);
            tft.fillRoundRect(90, 120, 300, 80, 8, TFT_WARNING);
            tft.setTextColor(TFT_TEXT);
            tft.setTextSize(1);
            tft.setCursor(110, 140);
            tft.println("Fehler beim Löschen des Versuchs!");
            tft.setCursor(110, 160);
            tft.println("Drücken Sie eine Taste, um fortzufahren...");
            
            // Warten auf Tastendruck
            bool warten2 = true;
            while (warten2) {
              char k2 = keypad.getKey();
              if (k2) {
                warten2 = false;
              }
              
              // Auch auf Encoder-Button prüfen
              if (digitalRead(ENCODER_BUTTON) == LOW) {
                if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
                  buttonPressed = true;
                  lastDebounceTime = millis();
                  warten2 = false;
                }
              } else {
                buttonPressed = false;
              }
            }
            
            // Zurück zur Liste der gespeicherten Versuche
            zeigeGespeicherteVersuche();
          }
        } else if (k == '*') {
          // Löschen abgebrochen
          warten = false;
          
          // Zurück zu den Versuchsdetails
          zeigeVersuchDetails(aktuellerVersuchsFilename);
        }
      }
    }
  }
}
 
 float WindTurbineExperiment::messeLeistung() {
   // Präzise Leistungsmessung durchführen
   float busvoltage = ina226.getBusVoltage();
   float shuntvoltage = ina226.getShuntVoltage_mV() / 1000.0; // Umrechnung von mV in V
   float current_mA = ina226.getCurrent_mA();
   
   // Power manuell berechnen statt die vom Sensor berechnete Leistung zu verwenden
   float power_calc_mW = busvoltage * current_mA; // V * A = mW
   float power_uW = abs(power_calc_mW) * 1000.0; // Umwandlung von mW in uW (nur positive Werte)
   
   // Debug-Ausgabe
   Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
   Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage * 1000.0); Serial.println(" mV");
   Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
   Serial.print("Power (calc):  "); Serial.print(power_calc_mW); Serial.println(" mW");
   Serial.print("Power (uW):    "); Serial.print(power_uW); Serial.println(" uW");
   Serial.println("");
   
   // Manuell berechnete Leistung in μW zurückgeben
   return power_uW;
 }

void WindTurbineExperiment::manuelleDatenLoeschung() {
  Serial.println("Manueller Reset gestartet");
  
  tft.fillScreen(TFT_BACKGROUND);
  
  // Warnung anzeigen
  tft.fillRoundRect(50, 60, 380, 200, 10, TFT_WARNING);
  tft.fillRoundRect(55, 65, 370, 190, 8, TFT_BACKGROUND);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WARNING);
  tft.setCursor(170, 90);
  tft.println("ACHTUNG!");
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(70, 120);
  tft.println("Sie sind dabei, ALLE gespeicherten Versuche zu loeschen!");
  tft.setCursor(70, 140);
  tft.println("Diese Aktion kann NICHT rueckgaengig gemacht werden.");
  
  tft.setTextColor(TFT_WARNING);
  tft.setCursor(70, 170);
  tft.println("Zum Fortfahren geben Sie die Sequenz ein: 1-2-3-4");
  tft.setCursor(70, 190);
  tft.println("Zum Abbrechen druecken Sie: D");
  
  tft.setTextColor(TFT_LIGHT_TEXT);
  tft.setCursor(70, 220);
  tft.println("Aktuelle Eingabe wird unten angezeigt:");
  
  // Eingabebereich
  tft.drawRect(70, 240, 300, 30, TFT_OUTLINE);
  
  // Bestätigungssequenz
  String eingabe = "";
  bool fertig = false;
  unsigned long letzteEingabe = millis();
  
  while (!fertig) {
    char key = keypad.getKey();
    
    // Timeout nach 30 Sekunden
    if (millis() - letzteEingabe > 30000) {
      Serial.println("Timeout bei manuellem Reset");
      fertig = true;
      zeigeIntro();
      return;
    }
    
    if (key) {
      letzteEingabe = millis();
      
      if (key == 'D') {
        // Abbrechen
        Serial.println("Manueller Reset abgebrochen");
        fertig = true;
        zeigeIntro();
        return;
      } else if (key >= '1' && key <= '4') {
        eingabe += key;
        
        // Eingabe anzeigen (mit Sternen für Sicherheit)
        tft.fillRect(72, 242, 296, 26, TFT_BACKGROUND);
        tft.setTextColor(TFT_TEXT);
        tft.setCursor(80, 250);
        tft.print("Eingabe: ");
        for (int i = 0; i < eingabe.length(); i++) {
          tft.print("*");
        }
        
        // Prüfe Sequenz
        if (eingabe == "1234") {
          fertig = true;
          
          Serial.println("Korrekte Sequenz eingegeben - führe Reset durch");
          
          // Löschung durchführen
          tft.fillScreen(TFT_BACKGROUND);
          tft.setTextSize(1);
          tft.setTextColor(TFT_WARNING);
          tft.setCursor(20, 120);
          tft.println("Loeschung wird durchgefuehrt - bitte warten...");
          
          // Fortschrittsbalken
          tft.drawRect(20, 160, 440, 20, TFT_OUTLINE);
          for (int i = 0; i <= 100; i += 10) {
            tft.fillRect(22, 162, (i * 436) / 100, 16, TFT_WARNING);
            delay(200);
          }
          
          if (dataManager.deleteAllExperiments()) {
            Serial.println("Manueller Reset erfolgreich");
            
            tft.fillScreen(TFT_BACKGROUND);
            tft.setTextSize(2);
            tft.setTextColor(TFT_SUCCESS);
            tft.setCursor(20, 120);
            tft.println("Alle Daten erfolgreich geloescht!");
            
            tft.setTextSize(1);
            tft.setTextColor(TFT_TEXT);
            tft.setCursor(20, 160);
            tft.println("Das System ist bereit fuer neue Experimente.");
            tft.setCursor(20, 180);
            tft.println("Druecken Sie eine Taste zum Fortfahren...");
            
            delay(2000);
          } else {
            Serial.println("Fehler beim manuellen Reset");
            
            tft.fillScreen(TFT_BACKGROUND);
            tft.setTextSize(2);
            tft.setTextColor(TFT_WARNING);
            tft.setCursor(20, 120);
            tft.println("Fehler beim Loeschen!");
            
            tft.setTextSize(1);
            tft.setTextColor(TFT_TEXT);
            tft.setCursor(20, 160);
            tft.println("Bitte wenden Sie sich an den Administrator.");
            
            delay(3000);
          }
          
          zeigeIntro();
        } else if (eingabe.length() >= 4) {
          // Falsche Sequenz
          Serial.println("Falsche Reset-Sequenz eingegeben");
          eingabe = "";
          
          tft.fillRect(70, 270, 300, 40, TFT_BACKGROUND);
          tft.setTextColor(TFT_WARNING);
          tft.setCursor(70, 275);
          tft.println("FALSCHE SEQUENZ!");
          tft.setCursor(70, 290);
          tft.println("Versuchen Sie es erneut oder druecken Sie D");
          
          // Eingabefeld zurücksetzen
          tft.fillRect(72, 242, 296, 26, TFT_BACKGROUND);
          
          delay(2000);
          
          // Fehlermeldung löschen
          tft.fillRect(70, 270, 300, 40, TFT_BACKGROUND);
        }
      } else {
        // Ungültige Taste
        tft.fillRect(70, 270, 300, 20, TFT_BACKGROUND);
        tft.setTextColor(TFT_LIGHT_TEXT);
        tft.setCursor(70, 275);
        tft.println("Nur Tasten 1-4 und D sind erlaubt");
        
        delay(1000);
        tft.fillRect(70, 270, 300, 20, TFT_BACKGROUND);
      }
    }
    
    delay(50);
  }
}

/**
 * Motor-Verbindungstest (Kern-Funktion)
 */
/**
 * Motor-Verbindungstest (Kern-Funktion) - AKKU-OPTIMIERT
 */
bool WindTurbineExperiment::testMotorVerbindung() {
  // Nur Test 1: PIN_A HIGH, PIN_B messen (funktioniert zuverlässig)
  pinMode(MOTOR_TEST_PIN_A, OUTPUT);
  pinMode(MOTOR_TEST_PIN_B, INPUT);
  
  digitalWrite(MOTOR_TEST_PIN_A, HIGH);
  delay(50);
  int analog1 = analogRead(MOTOR_TEST_PIN_B);
  
  // Pins aufräumen
  pinMode(MOTOR_TEST_PIN_A, OUTPUT);
  pinMode(MOTOR_TEST_PIN_B, OUTPUT);
  digitalWrite(MOTOR_TEST_PIN_A, LOW);
  digitalWrite(MOTOR_TEST_PIN_B, LOW);
  
  // Nur Test1 verwenden (zuverlässig bei USB + Akku)
  bool test1_ok = (analog1 > 1000);  // 0 ohne Motor, ~2700 mit Motor
  
  return test1_ok;
}

/**
 * Startup Motor-Check (nach Reset-Sequenz)
 */
void WindTurbineExperiment::startMotorStartupCheck() {
  Serial.println("=== MOTOR STARTUP-CHECK ===");
  
  // Kurze Pause für Hardware-Stabilisierung
  delay(500);
  
  // Motor-Test durchführen
  bool motorAngeschlossen = testMotorVerbindung();
  
  if (!motorAngeschlossen) {
    Serial.println("WARNUNG: Motor nicht angeschlossen beim Start");
    
    // Warnung anzeigen
    tft.fillScreen(TFT_BACKGROUND);
    
    // Warnungsbox
    tft.fillRoundRect(40, 80, 400, 160, 10, TFT_WARNING);
    tft.fillRoundRect(45, 85, 390, 150, 8, TFT_BACKGROUND);
    
    // Warnung-Icon
    tft.fillCircle(100, 140, 20, TFT_WARNING);
    tft.setTextSize(3);
    tft.setTextColor(TFT_BACKGROUND);
    tft.setCursor(92, 130);
    tft.print("!");
    
    // Warnung-Text
    tft.setTextSize(2);
    tft.setTextColor(TFT_WARNING);
    tft.setCursor(140, 110);
    tft.println("MOTOR-WARNUNG");
    
    tft.setTextSize(1);
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(140, 140);
    tft.println("Motor scheint nicht angeschlossen zu sein.");
    tft.setCursor(140, 160);
    tft.println("Moechten Sie trotzdem fortfahren?");
    
    // Optionen
    tft.setCursor(60, 200);
    tft.setTextColor(TFT_SUCCESS);
    tft.print("# = Weiter ohne Motor");
    
    tft.setCursor(60, 215);
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.print("* = Motor anschliessen und neu testen");
    
    // Auf Antwort warten
    bool warten = true;
    while (warten) {
      char key = keypad.getKey();
      
      if (key == '#') {
        // Ohne Motor weiter
        warten = false;
        Serial.println("User wählt: Weiter ohne Motor");
        
        // Monitoring starten (wird sofort Warnungen zeigen, aber das ist ok)
        letzterMotorCheck = millis();
        motorFehlerZaehler = 0;
        
      } else if (key == '*') {
        // Motor anschließen und neu testen
        
        // "Bitte warten" anzeigen
        tft.fillRect(60, 200, 320, 40, TFT_BACKGROUND);
        tft.setTextColor(TFT_HIGHLIGHT);
        tft.setCursor(60, 200);
        tft.print("Teste Verbindung...");
        
        delay(2000); // Dem User Zeit geben Motor anzuschließen
        
        if (testMotorVerbindung()) {
          // Erfolgreich!
          tft.fillRect(60, 200, 320, 40, TFT_BACKGROUND);
          tft.setTextColor(TFT_SUCCESS);
          tft.setCursor(60, 200);
          tft.print("Motor erfolgreich erkannt!");
          tft.setCursor(60, 215);
          tft.print("Weiter mit beliebiger Taste...");
          
          // Auf beliebige Taste warten
          bool warten2 = true;
          while (warten2) {
            char k = keypad.getKey();
            if (k) warten2 = false;
            
            if (digitalRead(ENCODER_BUTTON) == LOW) {
              if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
                buttonPressed = true;
                lastDebounceTime = millis();
                warten2 = false;
              }
            } else {
              buttonPressed = false;
            }
            delay(50);
          }
          
          warten = false;
          Serial.println("Motor erfolgreich angeschlossen");
          
          // Monitoring initialisieren
          letzterMotorCheck = millis();
          motorFehlerZaehler = 0;
          
        } else {
          // Immer noch nicht da
          tft.fillRect(60, 200, 320, 40, TFT_BACKGROUND);
          tft.setTextColor(TFT_WARNING);
          tft.setCursor(60, 200);
          tft.print("Motor immer noch nicht erkannt!");
          tft.setCursor(60, 215);
          tft.print("# = Trotzdem weiter, * = Nochmal");
          
          // Zurück zur Auswahl
        }
      }
      
      delay(50);
    }
  } else {
    Serial.println("Motor beim Start erfolgreich erkannt");
    
    // Erfolgs-Meldung kurz anzeigen
    tft.fillScreen(TFT_BACKGROUND);
    tft.fillRoundRect(120, 120, 240, 80, 10, TFT_SUCCESS);
    tft.setTextSize(2);
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(160, 145);
    tft.println("Motor OK!");
    
    tft.setTextSize(1);
    tft.setCursor(140, 170);
    tft.println("Verbindung erfolgreich");
    
    delay(1500);
    
    // Monitoring initialisieren
    letzterMotorCheck = millis();
    motorFehlerZaehler = 0;
  }
  
  Serial.println("Motor Startup-Check abgeschlossen");
  // Akku-Monitoring initialisieren
  pinMode(BATTERY_PIN, INPUT);
  akkuSpannung = messeAkkuSpannung();
  akkuProzent = berechneAkkuProzent(akkuSpannung);
  letzteAkkuPruefung = millis();
  
  Serial.print("Initiale Akkuspannung: ");
  Serial.print(akkuSpannung);
  Serial.print("V (");
  Serial.print(akkuProzent);
  Serial.println("%)");
}

/**
 * Background Motor-Monitoring (wird in loop() aufgerufen)
 */
void WindTurbineExperiment::handleMotorBackgroundMonitoring() {
  // Nur alle 10 Sekunden prüfen
  if (millis() - letzterMotorCheck < 10000) {
    return;
  }
  
  letzterMotorCheck = millis();
  
  // Monitoring pausiert?
  if (motorMonitoringPausiert) {
    if (millis() - motorWarningPauseStart > 60000) {
      // 60 Sekunden um, Monitoring wieder aktivieren
      motorMonitoringPausiert = false;
      motorFehlerZaehler = 0; // Fresh start
      Serial.println("Motor-Monitoring wieder aktiviert");
    } else {
      return; // Noch in Pause
    }
  }
  
  // Motor-Test durchführen
  bool motorDa = testMotorVerbindung();

  motorStatusAktuell = motorDa;
  
  if (motorDa) {
    // Motor ist da - Fehler-Counter zurücksetzen
    if (motorFehlerZaehler > 0) {
      Serial.println("Motor wieder angeschlossen - Fehler-Counter reset");
      motorFehlerZaehler = 0;
      
      // Falls Warnung aktiv war, verstecken
      if (motorWarnungAktiv) {
        versteckeMotorWarnung();
      }
    }
  } else {
    // Motor fehlt - Fehler-Counter erhöhen
    motorFehlerZaehler++;
    Serial.print("Motor-Fehler #");
    Serial.println(motorFehlerZaehler);
    
    // Nach 5 Fehlern Warnung anzeigen
    if (motorFehlerZaehler >= 5 && !motorWarnungAktiv) {
      Serial.println("5 Motor-Fehler erreicht - zeige Warnung");
      zeigeMotorWarnung();
    }
  }
}

/**
 * Motor-Warnung anzeigen (overlay)
 */
void WindTurbineExperiment::zeigeMotorWarnung() {
  motorWarnungAktiv = true;
  
  // Warnung als Overlay über aktuellen Bildschirm
  // Schatten-Effekt
  tft.fillRoundRect(52, 52, 376, 108, 8, 0x2104);
  
  // Hauptbox
  tft.fillRoundRect(50, 50, 376, 108, 8, TFT_WARNING);
  tft.fillRoundRect(55, 55, 366, 98, 6, TFT_BACKGROUND);
  
  // Warnung-Icon
  tft.fillCircle(85, 90, 15, TFT_WARNING);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BACKGROUND);
  tft.setCursor(79, 82);
  tft.print("!");
  
  // Warnung-Text
  tft.setTextSize(1);
  tft.setTextColor(TFT_WARNING);
  tft.setCursor(110, 70);
  tft.println("MOTOR-VERBINDUNG UNTERBROCHEN");
  
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(110, 90);
  tft.println("Bitte Motor-Anschluss pruefen");
  
  // Optionen
  tft.setTextColor(TFT_LIGHT_TEXT);
  tft.setCursor(110, 115);
  tft.print("* = 60s ignorieren");
  
  tft.setCursor(250, 115);
  tft.print("# = Neu testen");
  
  Serial.println("Motor-Warnung angezeigt");
}

/**
 * Motor-Warnung verstecken
 */
void WindTurbineExperiment::versteckeMotorWarnung() {
  if (!motorWarnungAktiv) return;
  
  motorWarnungAktiv = false;
  
  // Warnung-Bereich übermalen - einfach aktuellen Bildschirm neu zeichnen
  switch (aktuellerModus) {
    case INTRO:
      zeigeIntro();
      break;
    case TEILFAKTORIELL_PLAN:
      zeigeTeilfaktoriellPlan();
      break;
    case TEILFAKTORIELL_MESSUNG:
      zeigeTeilfaktoriellMessung();
      break;
    case TEILFAKTORIELL_AUSWERTUNG:
      zeigeTeilfaktoriellAuswertung();
      break;
    case VOLLFAKTORIELL_PLAN:
      zeigeVollfaktoriellPlan();
      break;
    case VOLLFAKTORIELL_MESSUNG:
      zeigeVollfaktoriellMessung();
      break;
    case VOLLFAKTORIELL_AUSWERTUNG:
      zeigeVollfaktoriellAuswertung();
      break;
    case REGRESSION:
      zeigeRegressionModell();
      break;
    case ZUSAMMENFASSUNG:
      zeigeZusammenfassung();
      break;
    default:
      // Fallback: schwarzer Bereich
      tft.fillRect(50, 50, 376, 108, TFT_BACKGROUND);
      break;
  }
  
  Serial.println("Motor-Warnung versteckt");
}


/**
 * Akku-Monitoring Funktionen
 */
void WindTurbineExperiment::handleAkkuMonitoring() {
  if (millis() - letzteAkkuPruefung > 30000) { // Alle 30 Sekunden
    akkuSpannung = messeAkkuSpannung();
    akkuProzent = berechneAkkuProzent(akkuSpannung);
    letzteAkkuPruefung = millis();
    
    // Warnung bei niedrigem Akku
    if (akkuProzent < 20) {
      Serial.println("⚠️ WARNUNG: Niedriger Akkustand!");
    }
  }
}

float WindTurbineExperiment::messeAkkuSpannung() {
  // Mehrere Messungen für Stabilität
  float summe = 0;
  for (int i = 0; i < 10; i++) {
    summe += analogRead(BATTERY_PIN);
    delay(10);
  }
  
  float adcWert = summe / 10.0;
  
  // ADC zu Spannung (ESP32: 12-bit ADC, 3.3V Referenz)
  float pinSpannung = (adcWert / 4095.0) * 3.3;
  
  // Zurückrechnen auf Akkuspannung
  float akkuSpannung = pinSpannung * VOLTAGE_DIVIDER_RATIO;
  
  return akkuSpannung;
}

int WindTurbineExperiment::berechneAkkuProzent(float spannung) {
  // Li-Ion Spannungskurve (vereinfacht)
  if (spannung >= 4.1) return 100;
  if (spannung >= 3.9) return 80 + (spannung - 3.9) * 100;
  if (spannung >= 3.7) return 40 + (spannung - 3.7) * 200;
  if (spannung >= 3.5) return 20 + (spannung - 3.5) * 100;
  if (spannung >= 3.2) return 5 + (spannung - 3.2) * 50;
  if (spannung >= 3.0) return (spannung - 3.0) * 25;
  
  return 0;
}