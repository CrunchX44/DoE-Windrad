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
 * 
 * Experimentelle Faktoren:
 * - Steigung (4 und 6)
 * - Groesse (7 und 8 Zoll)
 * - Abstand (30 und 60 cm)
 * - Luftstaerke (1 und 3)
 * - Blattanzahl (2 und 3)
 * 
 * Dateistruktur:
 * - WindTurbineExperiment.h: Hauptheader mit Klassendeklaration
 * - WindTurbineConstants.h: Konstanten und Definitionen
 * - WindTurbineExperiment.cpp: Kernfunktionalität
 * - WindTurbineUI.cpp: Benutzeroberfläche und Anzeigefunktionen
 * - WindTurbineCalculations.cpp: Berechnungsmethoden
 * - WindTurbineVisualizations.cpp: Diagramme und Visualisierungen
 * - WindTurbineDataManager.h: Datenverwaltung Header
 * - WindTurbineDataManager.cpp: Datenverwaltung Implementierung
 * - WindTurbineDataUI.cpp: UI für Datenverwaltung und Export
 */

 #include "WindTurbineExperiment.h"

 // Hauptobjekt erstellen
 WindTurbineExperiment experiment;
 
 void setup() {
   // Initialisierung an das Hauptobjekt delegieren
   experiment.setup();
 }
 
 void loop() {
   // Hauptschleife an das Hauptobjekt delegieren
   experiment.loop();
 }
