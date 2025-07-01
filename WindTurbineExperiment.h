/**
 * Design of Experiments (DoE) für Windkraftanlage
 * ESP32-Anwendung zur Durchführung von teil- und vollfaktoriellen Versuchen
 * 
 * Hardwarekomponenten:
 * - ESP32
 * - TFT-Display (480x320)
 * - INA226 Leistungssensor
 * - Encoder (Drehregler)
 * - Keypad (4x4)
 */

#ifndef WIND_TURBINE_EXPERIMENT_H
#define WIND_TURBINE_EXPERIMENT_H

#include <Wire.h>              // I2C-Kommunikation
#include <INA226.h>            // Leistungssensor (RobTillaart Bibliothek)
#include <SPI.h>               // SPI-Kommunikation
#include <Keypad.h>            // Nummernblock
#include <ESP32Encoder.h>      // Drehregler
#include <TFT_eSPI.h>          // Display
#include <SPIFFS.h>            // Dateisystem fuer Datenspeicherung
#include <rom/rtc.h>           // ESP32 Reset Reason API (fallback)
#include <esp_system.h>        // Für esp_reset_reason() (moderne ESP32 API)
#include "WindTurbineConstants.h"
#include "WindTurbineDataManager.h"

// Motor-Verbindungstest Pins
#define MOTOR_TEST_PIN_A 12
#define MOTOR_TEST_PIN_B 33

// Battery monitoring
#define BATTERY_PIN 0           // ADC Pin für Akkumessung
#define VOLTAGE_DIVIDER_RATIO 1.961  // 22kΩ / 12kΩ = 1.961

class WindTurbineExperiment {
public:
  // Konstruktor
  WindTurbineExperiment();
  
  // Hauptfunktionen
  void setup();
  void loop();

private:
  // Status-Enum
  enum ProgrammModus {
    INTRO,
    TEILFAKTORIELL_PLAN,
    TEILFAKTORIELL_MESSUNG,
    TEILFAKTORIELL_AUSWERTUNG,
    VOLLFAKTORIELL_PLAN,
    VOLLFAKTORIELL_MESSUNG,
    VOLLFAKTORIELL_AUSWERTUNG,
    REGRESSION,
    ZUSAMMENFASSUNG,
    BESTAETIGUNG_DIALOG, // Neuer Modus für Bestätigungsdialoge
    GESPEICHERTE_VERSUCHE, // Modus für die Anzeige gespeicherter Versuche
    VERSUCH_DETAILS, // Modus für die Detailansicht eines gespeicherten Versuchs
    WIFI_EXPORT, // Modus für den WiFi-Export
    BESCHREIBUNG_EINGABE // Modus für die Eingabe einer Versuchsbeschreibung
  };

  // Objektreferenzen
  Keypad keypad;
  TFT_eSPI tft;
  INA226 ina226 = INA226(0x40); // Standard I2C-Adresse für INA226
  ESP32Encoder encoder;
  WindTurbineDataManager dataManager;

  // Statusvariablen
  ProgrammModus aktuellerModus;
  ProgrammModus vorherigerModus; // Für Zurück-Funktion
  ProgrammModus naechsterModus;  // Für Bestätigungsdialoge
  int aktuellerVersuch;
  float teilfaktoriellMessungen[8][5]; // 8 Versuche × 5 Messwerte
  float teilfaktoriellMittelwerte[8];
  float teilfaktoriellStandardabweichungen[8];
  float vollfaktoriellMessungen[8][5]; // 8 Versuche × 5 Messwerte
  float vollfaktoriellMittelwerte[8];
  float vollfaktoriellStandardabweichungen[8];
  float effekte[5]; // Haupteffekte für 5 Faktoren
  int aktuelleMessung;
  int ausgewaehlteVollfaktoren[3]; // Beispiel: Steigung, Abstand, Blattanzahl
  
  // NEUE VARIABLE: Fixierte Faktorwerte für nicht ausgewählte Faktoren
  int fixierteFaktorwerte[5]; // 99 = nicht fixiert, -1 = niedrige Stufe, 1 = hohe Stufe
  
  // Datenverwaltungs-Variablen
  char versuchsBeschreibung[100]; // Beschreibung für gespeicherte Versuche
  char aktuellerVersuchsFilename[50]; // Dateiname des aktuell ausgewählten Versuchs
  ExperimentMetadata gespeicherteVersuche[MAX_SAVED_EXPERIMENTS]; // Liste gespeicherter Versuche
  int anzahlGespeicherteVersuche; // Anzahl der gespeicherten Versuche
  char textEingabe[100]; // Puffer für Texteingaben

  // Variablen für die UI-Steuerung
  int encoderPosition;
  int previousEncoderPosition;
  int cursorPosition;
  int maxCursorPosition;
  bool buttonPressed;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;

  // Motor-Monitoring Variablen
  unsigned long letzterMotorCheck;
  int motorFehlerZaehler;
  bool motorWarnungAktiv;
  bool motorMonitoringPausiert;
  unsigned long motorWarningPauseStart;
  bool motorStatusAktuell;
  // Battery monitoring
  unsigned long letzteAkkuPruefung;
  float akkuSpannung;
  int akkuProzent;

  // UI-Hilfsfunktionen
  void zeichneTitelbalken(const char* titel);
  void zeichneStatusleiste(const char* status);
  void zeichneMotorStatusBox();
  void zeigeBestaetigung(const char* nachricht, ProgrammModus zielModus);
  void zurueckZumVorherigenModus();
  void zeigeFeedback(bool korrekt, float eingabe, float korrekterWert, const char* einheit, const char* kategorie);

  // UI-Funktionen
  void zeigeIntro();
  void zeigeTeilfaktoriellPlan();
  void zeigeTeilfaktoriellMessung();
  void zeigeTeilfaktoriellAuswertung();
  void zeigeVollfaktoriellPlan();
  void zeigeVollfaktoriellMessung();
  void zeigeVollfaktoriellAuswertung();
  void zeigeRegressionModell();
  void zeigeZusammenfassung();
  
  // Datenverwaltungs-UI-Funktionen
  void zeigeGespeicherteVersuche();
  void zeigeVersuchDetails(const char* filename);
  void zeigeBeschreibungEingabe();
  void zeigeWiFiExport();
  
  // Event-Handler
  void aktualisiereUI();
  void verarbeiteButtonDruck();
  void verarbeiteKeypadEingabe(char key);
  
  // Messfunktionen
  float messeLeistung();
  void manuelleMittelwertEingabe(bool istTeilfaktoriell, int versuchIndex = 0, bool zurueckZurAuswertung = false);
  void manuelleStandardabweichungEingabe(bool istTeilfaktoriell, int versuchIndex = 0, bool zurueckZurAuswertung = false);
  void manuelleEffektBerechnung();
  
  // Berechnungsfunktionen
  float berechneMittelwert(float* messungen, int anzahl);
  float berechneStandardabweichung(float* messungen, int anzahl, float mittelwert);
  void berechneEffekte();
  void bestimmeWichtigsteFaktoren();
  float berechneRegressionsKoeffizient(int koeffIndex);
  float berechneR2();
  float berechnePrognose();
  
  // Visualisierungsfunktionen
  void zeigeHaupteffekteDiagrammAnsicht();
  void zeigeInteraktionsDiagrammAnsicht();
  void zeigeTeilfaktoriellDiagrammAnsicht();
  void zeigeVollfaktoriellDiagrammAnsicht();
  void zeigeParetoEffekteDiagrammAnsicht();
  void zeigeHaupteffekteDiagramm();
  void zeigeInteraktionsDiagramm();
  void zeigeEffekteDiagramm(int x, int y);
  void zeigeVollfaktoriellDiagramm(int x, int y);
  void zeigeParetoEffekteDiagramm(int x, int y);
  
  // Reset-Funktionalität
  void manuelleDatenLoeschung();

  // Motor-Test Funktionen
  bool testMotorVerbindung();
  void startMotorStartupCheck();
  void handleMotorBackgroundMonitoring();
  void zeigeMotorWarnung();
  void versteckeMotorWarnung();
  // Battery monitoring functions
  void handleAkkuMonitoring();
  float messeAkkuSpannung();
  int berechneAkkuProzent(float spannung);
};

#endif // WIND_TURBINE_EXPERIMENT_H
