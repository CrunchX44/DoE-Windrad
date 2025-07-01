/**
 * WindTurbineDataManager.h
 * ERWEITERTE Schnittstelle für die Datenverwaltung des Windkraftanlagen-Experiments
 * 
 * NEUE FEATURES:
 * - Korrigierte PNG-Export-Funktionen für alle Diagramme
 * - SVG-Export für verlustfreie Skalierung
 * - Batch-Export für alle Diagramme
 * - Hochauflösende Exports (300 DPI)
 * - Metadaten-Einbettung
 */

#ifndef WIND_TURBINE_DATA_MANAGER_H
#define WIND_TURBINE_DATA_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "WindTurbineConstants.h"

// Struktur für Metadaten gespeicherter Experimente
struct ExperimentMetadata {
  char filename[50];
  char description[100];
  char timestamp[20];
  float maxPower;
  bool isValid;
};

// NEUE STRUKTUREN für erweiterte Export-Funktionen
struct ChartExportConfig {
  int width;
  int height;
  int dpi;
  bool includeMetadata;
  bool highQuality;
  String format; // "png", "svg", "both"
};

struct BatchExportProgress {
  int totalCharts;
  int completedCharts;
  String currentChart;
  bool isComplete;
  String errorMessage;
};

class WindTurbineDataManager {
public:
  // Konstruktor und Destruktor
  WindTurbineDataManager();
  ~WindTurbineDataManager();
  
  // Initialisierung
  bool begin();
  
  // Datenverwaltungsfunktionen (bestehend)
  bool saveExperiment(const char* description, float teilfaktoriellMessungen[][5], 
                     float teilfaktoriellMittelwerte[], float teilfaktoriellStandardabweichungen[],
                     float vollfaktoriellMessungen[][5], float vollfaktoriellMittelwerte[], 
                     float vollfaktoriellStandardabweichungen[], float effekte[], 
                     int ausgewaehlteVollfaktoren[]);
  
  bool loadExperiment(const char* filename, float teilfaktoriellMessungen[][5], 
                     float teilfaktoriellMittelwerte[], float teilfaktoriellStandardabweichungen[],
                     float vollfaktoriellMessungen[][5], float vollfaktoriellMittelwerte[], 
                     float vollfaktoriellStandardabweichungen[], float effekte[], 
                     int ausgewaehlteVollfaktoren[]);
  
  int listExperiments(ExperimentMetadata* metadata, int maxCount);
  bool deleteExperiment(const char* filename);
  bool deleteAllExperiments();
  
  // Export-Funktionen (bestehend)
  bool startWiFiExport(const char* filename);
  void stopWiFiExport();
  void handleWiFiExport();
  
  // Hilfsfunktionen (bestehend)
  String getExportURL();
  bool isExportActive();
  String getCurrentSSID();
  bool isWiFiExportActive();
  String getCurrentIP();
  
  // Speicher-Verwaltung (bestehend)
  size_t getUsedSpace();
  size_t getTotalSpace();
  size_t getFreeSpace();
  
  // Erweiterte Datei-Funktionen (bestehend)
  bool fileExists(const char* filename);
  size_t getFileSize(const char* filename);
  
  // Datenvalidierung (bestehend)
  bool validateExperimentData(float teilfaktoriellMessungen[][5], 
                             float vollfaktoriellMessungen[][5]);
  
  // Backup und Restore (bestehend)
  bool createBackup(const char* backupName);
  bool restoreBackup(const char* backupFilename);
  
  // System-Diagnose (bestehend)
  void printSystemInfo();
  
  // === NEUE ERWEITERTE EXPORT-FUNKTIONEN ===
  
  // Einzeldiagramm-Export (korrigiert)
  void serveCorrectedPNG(const String& chartType, const char* filename);
  void serveCorrectedSVG(const String& chartType, const char* filename);
  
  // Batch-Export
  void serveAllChartsAsZip(const char* filename);
  void startBatchExport(const char* filename, const ChartExportConfig& config);
  BatchExportProgress getBatchExportProgress();
  
  // Hochauflösende Exports
  void serveHDPNG(const String& chartType, const char* filename, int width = 1200, int height = 900);
  void servePublicationQualityExport(const char* filename); // 300 DPI, alle Formate
  
  // Konfigurierbare Exports
  void serveConfigurableExport(const String& chartType, const char* filename, 
                              const ChartExportConfig& config);
  
  // Metadaten-Export
  String generateChartMetadata(const String& chartType, const char* filename);
  void embedMetadataInExport(const String& chartType, const char* filename);

private:
  // Dateisystem-Funktionen (bestehend)
  bool initSPIFFS();
  String generateFilename();
  
  // WiFi-Export-Variablen (bestehend)
  WebServer* server;
  bool wifiExportActive;
  String currentExportFilename;
  String currentSSID;
  
  // === NEUE PRIVATE FUNKTIONEN FÜR ERWEITERTE EXPORTS ===
  
  // Basis-Funktionen (bestehend, teilweise erweitert)
  void serveFileChunked(const char* filename);
  void serveJSONChunked(const char* filename);
  void serveEnhancedIndex();
  void serveAdvancedGraphics(const char* filename);
  void serveCompleteCSV(const char* filename);
  void serveCompletePackage(const char* filename);
  
  // KORRIGIERTE Chart-Generierung
  void generateCorrectedMainEffectsJS(const char* filename);
  void generateCorrectedParetoJS(const char* filename);
  void generateCorrectedInteractionJS(const char* filename);
  void generateFactorialChartJS(const char* filename);
  void generateEffectsChartJS(const char* filename);
  
  // SVG-Generierung
  void generateMainEffectsSVG(const char* filename);
  void generateParetoSVG(const char* filename);
  void generateInteractionSVG(const char* filename);
  void generateFactorialSVG(const char* filename);
  void generateEffectsSVG(const char* filename);
  
  // Utilitätsfunktionen für Diagramme
  String loadExperimentDataAsJSON(const char* filename);
  void calculateMainEffectsData(const char* filename, float* meanLow, float* meanHigh, 
                               float& overallMean, float& minResponse, float& maxResponse);
  void calculateInteractionData(const char* filename, int factor1, int factor2, 
                               float data[4], bool dataAvailable[4]);
  void calculateParetoData(const char* filename, float* effects, int* sortedIndices, 
                          float* percentages, float* cumulative);
  
  // Export-Konfiguration
  ChartExportConfig defaultConfig;
  BatchExportProgress batchProgress;
  
  // Metadaten-Verwaltung
  void addExperimentMetadata(String& content, const char* filename);
  String formatTimestamp();
  String getChartTitle(const String& chartType);
  String getChartDescription(const String& chartType);
  
  // Hilfsfunktionen für mathematische Korrektheit
  void validateChartData(const String& chartType, const char* filename);
  bool hasCompleteFactorialData(const char* filename, int factor1, int factor2);
  void interpolateMissingData(float data[4], bool available[4]);
  
  // Performance-Optimierung
  void optimizeExportSize();
  void cleanupTempFiles();
};

#endif // WIND_TURBINE_DATA_MANAGER_H
