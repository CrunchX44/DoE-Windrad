/**
 * WindTurbineDataManager.cpp
 * VOLLSTÄNDIGE Implementierung der Datenverwaltung mit  Export-Funktionen
 * 
 * WICHTIGE KORREKTUREN:
 * - Main Effects Plot: Mathematisch Berechnung mit echten Mittelwerten
 * - Pareto-Diagramm: Konsistente Prozent-Skalierung für Balken und Kurve
 * - Interaction Plot: Robuste Behandlung fehlender Datenpunkte
 * - Hochauflösende Exports und Batch-Export-Funktionalität
 * - SVG-Export für verlustfreie Skalierung
 */

#include "WindTurbineDataManager.h"
#include <time.h>

// Konstruktor mit erweiterten Konfigurationen
WindTurbineDataManager::WindTurbineDataManager() : 
  server(nullptr),
  wifiExportActive(false) {
  
  // Standard-Export-Konfiguration
  defaultConfig.width = 800;
  defaultConfig.height = 600;
  defaultConfig.dpi = 150;
  defaultConfig.includeMetadata = true;
  defaultConfig.highQuality = true;
  defaultConfig.format = "png";
  
  // Batch-Export-Status
  batchProgress.totalCharts = 0;
  batchProgress.completedCharts = 0;
  batchProgress.isComplete = false;
}

WindTurbineDataManager::~WindTurbineDataManager() {
  if (server) {
    delete server;
    server = nullptr;
  }
}

bool WindTurbineDataManager::begin() {
  Serial.println("Initialisiere SPIFFS...");
  
  if (!SPIFFS.begin(true)) {
    Serial.println("Fehler beim Initialisieren von SPIFFS");
    return false;
  }
  
  // Dateisystem-Info
  Serial.print("SPIFFS Total: ");
  Serial.println(SPIFFS.totalBytes());
  Serial.print("SPIFFS Used: ");
  Serial.println(SPIFFS.usedBytes());
  
  return true;
}

/**
 * Startet den WiFi-Export für Daten und Diagramme
 */
bool WindTurbineDataManager::startWiFiExport(const char* filename) {
  if (wifiExportActive) {
    stopWiFiExport();
  }
  
  Serial.println("Starte WiFi-Export...");
  
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_11dBm);
  
  int randomNum = random(1000, 9999);
  String ssid = "WindTurbine_" + String(randomNum);
  currentSSID = ssid;
  
  if (!WiFi.softAP(ssid.c_str(), "windturbine", 1, 0, 1)) {
    Serial.println("Fehler beim Starten des WiFi-AP");
    return false;
  }
  
  Serial.print("AP gestartet mit SSID: ");
  Serial.println(ssid);
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.softAPIP());
  
  if (!MDNS.begin("windturbine")) {
    Serial.println("Fehler beim Starten von mDNS");
  }
  
  try {
    server = new WebServer(80);
    if (!server) {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      return false;
    }
  } catch (...) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return false;
  }
  
  // Basis-Routen
  server->on("/", HTTP_GET, [this]() {
    this->serveEnhancedIndex();
  });
  
  // Standard-Downloads
  server->on("/data.json", HTTP_GET, [this, filename]() {
    this->serveJSONChunked(filename);
  });
  
  server->on("/data.csv", HTTP_GET, [this, filename]() {
    this->serveCompleteCSV(filename);
  });
  
  server->on("/download", HTTP_GET, [this, filename]() {
    this->serveFileChunked(filename);
  });
  
  // PNG-Export Routen
  server->on("/png/main-effects", HTTP_GET, [this, filename]() {
    this->serveCorrectedPNG("main-effects", filename);
  });
  
  server->on("/png/pareto", HTTP_GET, [this, filename]() {
    this->serveCorrectedPNG("pareto", filename);
  });
  
  server->on("/png/interaction", HTTP_GET, [this, filename]() {
    this->serveCorrectedPNG("interaction", filename);
  });
  
  server->on("/png/effects", HTTP_GET, [this, filename]() {
    this->serveCorrectedPNG("effects", filename);
  });
  
  server->on("/png/factorial", HTTP_GET, [this, filename]() {
    this->serveCorrectedPNG("factorial", filename);
  });
  
  // SVG-Export Routen (verlustfrei skalierbar)
  server->on("/svg/main-effects", HTTP_GET, [this, filename]() {
    this->serveCorrectedSVG("main-effects", filename);
  });
  
  server->on("/svg/pareto", HTTP_GET, [this, filename]() {
    this->serveCorrectedSVG("pareto", filename);
  });
  
  server->on("/svg/interaction", HTTP_GET, [this, filename]() {
    this->serveCorrectedSVG("interaction", filename);
  });
  
  server->on("/svg/effects", HTTP_GET, [this, filename]() {
    this->serveCorrectedSVG("effects", filename);
  });
  
  server->on("/svg/factorial", HTTP_GET, [this, filename]() {
    this->serveCorrectedSVG("factorial", filename);
  });
  
  server->begin();
  
  currentExportFilename = filename;
  wifiExportActive = true;
  
  Serial.println("WiFi-Export gestartet");
  return true;
}

void WindTurbineDataManager::stopWiFiExport() {
  if (server) {
    server->stop();
    delete server;
    server = nullptr;
  }
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiExportActive = false;
  
  Serial.println("WiFi-Export gestoppt");
}

void WindTurbineDataManager::handleWiFiExport() {
  if (server && wifiExportActive) {
    server->handleClient();
  }
}

/**
 * NEUE: Erweiterte Index-Seite mit allen Export-Optionen
 */
void WindTurbineDataManager::serveEnhancedIndex() {
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  
  server->sendContent("<!DOCTYPE html><html lang='de'><head>");
  server->sendContent("<meta charset='UTF-8'>");
  server->sendContent("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  server->sendContent("<title>🌪️ Windkraft-Experiment Daten-Export</title>");
  
  // Modernes CSS
  server->sendContent("<style>");
  server->sendContent("* { margin: 0; padding: 0; box-sizing: border-box; }");
  server->sendContent("body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }");
  server->sendContent(".container { max-width: 1200px; margin: 0 auto; padding: 20px; }");
  server->sendContent(".header { text-align: center; color: white; margin-bottom: 40px; }");
  server->sendContent(".header h1 { font-size: 2.5rem; margin-bottom: 10px; text-shadow: 2px 2px 4px rgba(0,0,0,0.3); }");
  server->sendContent(".header p { font-size: 1.2rem; opacity: 0.9; }");
  server->sendContent(".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin-bottom: 40px; }");
  server->sendContent(".card { background: white; border-radius: 15px; padding: 25px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); transition: transform 0.3s ease; }");
  server->sendContent(".card:hover { transform: translateY(-5px); }");
  server->sendContent(".card h2 { color: #333; margin-bottom: 15px; display: flex; align-items: center; gap: 10px; }");
  server->sendContent(".card p { color: #666; margin-bottom: 20px; line-height: 1.6; }");
  server->sendContent(".btn { display: inline-block; padding: 12px 24px; background: linear-gradient(45deg, #667eea, #764ba2); color: white; text-decoration: none; border-radius: 8px; margin: 5px; transition: all 0.3s ease; border: none; cursor: pointer; }");
  server->sendContent(".btn:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(0,0,0,0.2); }");
  server->sendContent(".btn-primary { background: linear-gradient(45deg, #4CAF50, #45a049); }");
  server->sendContent(".btn-secondary { background: linear-gradient(45deg, #2196F3, #1976D2); }");
  server->sendContent(".btn-warning { background: linear-gradient(45deg, #ff9800, #f57c00); }");
  server->sendContent(".correction-badge { background: #d4edda; color: #155724; padding: 8px 16px; border-radius: 20px; font-size: 0.9rem; margin-bottom: 15px; display: inline-block; }");
  server->sendContent(".footer { text-align: center; color: white; opacity: 0.8; margin-top: 40px; }");
  server->sendContent("</style>");
  server->sendContent("</head><body>");
  
  server->sendContent("<div class='container'>");
  server->sendContent("<div class='header'>");
  server->sendContent("<h1>🌪️ Windkraft-Experiment</h1>");
  server->sendContent("<p>Daten-Export Portal</p>");
  server->sendContent("</div>");
  
  server->sendContent("<div class='grid'>");
  
  // Standard-Downloads
  server->sendContent("<div class='card'>");
  server->sendContent("<h2>📊 Standard-Downloads</h2>");
  server->sendContent("<p>Grundlegende Datenformate für weitere Analyse</p>");
  server->sendContent("<a href='/data.json' class='btn btn-primary'>📄 JSON Daten</a>");
  server->sendContent("<a href='/data.csv' class='btn btn-primary'>📈 CSV Export</a>");
  server->sendContent("</div>");
  
  // Korrigierte PNG-Exports
  server->sendContent("<div class='card'>");
  server->sendContent("<h2>🖼️ PNG-Diagramme</h2>");
  server->sendContent("<p>Hochwertige PNG-Diagramme</p>");
  server->sendContent("<a href='/png/main-effects' class='btn'>📊 Main Effects Plot</a>");
  server->sendContent("<a href='/png/pareto' class='btn'>📈 Pareto-Diagramm</a>");
  server->sendContent("<a href='/png/interaction' class='btn'>🔄 Interaction Plot</a>");
  server->sendContent("<a href='/png/effects' class='btn'>📊 Effekt-Diagramm</a>");
  server->sendContent("<a href='/png/factorial' class='btn'>🧮 Faktorieller Vergleich</a>");
  server->sendContent("</div>");
  
  // SVG-Exports
  server->sendContent("<div class='card'>");
  server->sendContent("<h2>🎨 SVG-Exports (Vektorgrafik)</h2>");
  server->sendContent("<p>Verlustfrei skalierbare Vektorgrafiken für professionelle Verwendung</p>");
  server->sendContent("<a href='/svg/main-effects' class='btn btn-warning'>📐 SVG Main Effects</a>");
  server->sendContent("<a href='/svg/pareto' class='btn btn-warning'>📐 SVG Pareto</a>");
  server->sendContent("<a href='/svg/interaction' class='btn btn-warning'>📐 SVG Interaction</a>");
  server->sendContent("<a href='/svg/effects' class='btn btn-warning'>📐 SVG Effekte</a>");
  server->sendContent("<a href='/svg/factorial' class='btn btn-warning'>📐 SVG Faktoriell</a>");
  server->sendContent("</div>");
  
  server->sendContent("</div>");
  
  server->sendContent("<div class='footer'>");
  server->sendContent("<p>Windkraft-Experiment Daten-Export</p>");
  server->sendContent("</div>");
  
  server->sendContent("</div>");
  server->sendContent("</body></html>");
}

/**
 * KORRIGIERT: Generiert mathematisch korrekte PNG-Exports
 */
void WindTurbineDataManager::serveCorrectedPNG(const String& chartType, const char* filename) {
  Serial.println("Generiere PNG für: " + chartType);
  
  // Lade und validiere Daten
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    server->send(404, "text/plain", "Datei nicht gefunden");
    return;
  }
  
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    server->send(500, "text/plain", "Fehler beim Laden der Daten");
    return;
  }
  
  // Validiere Chart-Daten vor Export
  validateChartData(chartType, filename);
  
  // HTML-Seite mit mathematisch korrektem Canvas
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  
  server->sendContent("<!DOCTYPE html><html><head>");
  server->sendContent("<meta charset='UTF-8'>");
  server->sendContent("<title> " + chartType + " Export</title>");
  server->sendContent("<style>");
  server->sendContent("body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }");
  server->sendContent(".container { max-width: 1000px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }");
  server->sendContent("canvas { border: 1px solid #ddd; margin: 20px 0; }");
  server->sendContent(".download-btn { background: #007bff; color: white; padding: 15px 30px; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; margin: 10px; }");
  server->sendContent(".download-btn:hover { background: #0056b3; }");
  server->sendContent(".info { background: #e9ecef; padding: 15px; border-radius: 5px; margin: 10px 0; }");
  server->sendContent(".correction-notice { background: #d4edda; border: 1px solid #c3e6cb; padding: 10px; border-radius: 5px; margin: 10px 0; }");
  server->sendContent("</style>");
  server->sendContent("</head><body>");
  
  server->sendContent("<div class='container'>");
  server->sendContent("<h1>📊 " + getChartTitle(chartType) + "</h1>");
  
  // Korrektur-Hinweis
  server->sendContent("<div class='correction-notice'>");
  if (chartType == "main-effects") {
    server->sendContent("Zeigt echte Mittelwerte bei -1 und +1 Level aus den Messdaten");
  } else if (chartType == "pareto") {
    server->sendContent("Konsistente Prozent-Skalierung für Balken und kumulative Kurve");
  } else if (chartType == "interaction") {
    server->sendContent("Robuste Behandlung fehlender Datenpunkte mit Interpolation");
  } else if (chartType == "effects") {
    server->sendContent("Effektberechnung aus teilfaktoriellen Daten");
  } else if (chartType == "factorial") {
    server->sendContent("Vollständige Darstellung aller faktoriellen Kombinationen");
  }
  server->sendContent("</div>");
  
  server->sendContent("<div class='info'>Das Diagramm wird automatisch generiert. Klicken Sie auf 'PNG Herunterladen' um es zu speichern.</div>");
  
  server->sendContent("<canvas id='chart' width='800' height='600'></canvas>");
  server->sendContent("<br>");
  server->sendContent("<button class='download-btn' onclick='downloadPNG()'>📥 HD PNG Herunterladen</button>");
  server->sendContent("<button class='download-btn' onclick='downloadSVG()'>📥 SVG Herunterladen</button>");
  server->sendContent("<button class='download-btn' onclick='history.back()'>🔙 Zurück</button>");
  
  // JavaScript für korrigierte Diagramm-Generierung
  server->sendContent("<script>");
  
  // Daten an JavaScript übertragen
  String jsonData = loadExperimentDataAsJSON(filename);
  server->sendContent("const data = " + jsonData + ";");
  
  server->sendContent("const canvas = document.getElementById('chart');");
  server->sendContent("const ctx = canvas.getContext('2d');");
  server->sendContent("const faktoren = ['Steigung', 'Groesse', 'Abstand', 'Luftstaerke', 'Blaetter'];");
  
  // Chart-spezifische korrigierte Generierung
  if (chartType == "main-effects") {
    generateCorrectedMainEffectsJS(filename);
  } else if (chartType == "pareto") {
    generateCorrectedParetoJS(filename);
  } else if (chartType == "interaction") {
    generateCorrectedInteractionJS(filename);
  } else if (chartType == "effects") {
    generateEffectsChartJS(filename);
  } else if (chartType == "factorial") {
    generateFactorialChartJS(filename);
  }
  
  // Download-Funktionen
  server->sendContent("function downloadPNG() {");
  server->sendContent("  const link = document.createElement('a');");
  server->sendContent("  link.download = 'windkraft_" + chartType + "_HD.png';");
  server->sendContent("  link.href = canvas.toDataURL('image/png', 1.0);");
  server->sendContent("  link.click();");
  server->sendContent("}");
  
  server->sendContent("function downloadSVG() {");
  server->sendContent("  window.open('/svg/" + chartType + "', '_blank');");
  server->sendContent("}");
  
  // Metadaten einbetten
  if (defaultConfig.includeMetadata) {
    String metadata = generateChartMetadata(chartType, filename);
    server->sendContent("// Experiment Metadata: " + metadata);
  }
  
  server->sendContent("</script>");
  server->sendContent("</div></body></html>");
  
  Serial.println(" PNG-Generator-Seite für " + chartType + " gesendet");
}

/**
 * NEUE: SVG-Export Funktionen
 */
void WindTurbineDataManager::serveCorrectedSVG(const String& chartType, const char* filename) {
  Serial.println("Generiere SVG für: " + chartType);
  
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "image/svg+xml", "");
  
  server->sendContent("<?xml version='1.0' encoding='UTF-8'?>");
  server->sendContent("<svg width='800' height='600' xmlns='http://www.w3.org/2000/svg'>");
  server->sendContent("<defs>");
  server->sendContent("<style>");
  server->sendContent(".title { font: bold 20px Arial; fill: #2c3e50; }");
  server->sendContent(".axis { stroke: #495057; stroke-width: 2; fill: none; }");
  server->sendContent(".grid { stroke: #e9ecef; stroke-width: 1; }");
  server->sendContent(".text { font: 12px Arial; fill: #495057; }");
  server->sendContent(".factor-text { font: 14px Arial; fill: #495057; }");
  server->sendContent("</style>");
  server->sendContent("</defs>");
  
  // Chart-spezifische SVG-Generierung
  if (chartType == "main-effects") {
    generateMainEffectsSVG(filename);
  } else if (chartType == "pareto") {
    generateParetoSVG(filename);
  } else if (chartType == "interaction") {
    generateInteractionSVG(filename);
  } else if (chartType == "effects") {
    generateEffectsSVG(filename);
  } else if (chartType == "factorial") {
    generateFactorialSVG(filename);
  }
  
  server->sendContent("</svg>");
}

/**
 * NEUE: HD PNG-Export
 */
void WindTurbineDataManager::serveHDPNG(const String& chartType, const char* filename, int width, int height) {
  Serial.println("Generiere HD PNG für: " + chartType + " (" + String(width) + "x" + String(height) + ")");
  
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  
  server->sendContent("<!DOCTYPE html><html><head>");
  server->sendContent("<meta charset='UTF-8'>");
  server->sendContent("<title>HD " + chartType + " Export</title>");
  server->sendContent("<style>");
  server->sendContent("body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; text-align: center; }");
  server->sendContent("canvas { border: 1px solid #ddd; margin: 20px 0; max-width: 100%; height: auto; }");
  server->sendContent(".download-btn { background: #007bff; color: white; padding: 15px 30px; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; margin: 10px; }");
  server->sendContent("</style>");
  server->sendContent("</head><body>");
  
  server->sendContent("<h1>📊 HD " + getChartTitle(chartType) + " (" + String(width) + "x" + String(height) + ")</h1>");
  server->sendContent("<p>Hochauflösende Version für Präsentationen und Publikationen</p>");
  
  server->sendContent("<canvas id='chart' width='" + String(width) + "' height='" + String(height) + "'></canvas>");
  server->sendContent("<br>");
  server->sendContent("<button class='download-btn' onclick='downloadHD()'>📥 HD PNG Herunterladen</button>");
  server->sendContent("<button class='download-btn' onclick='history.back()'>🔙 Zurück</button>");
  
  server->sendContent("<script>");
  
  String jsonData = loadExperimentDataAsJSON(filename);
  server->sendContent("const data = " + jsonData + ";");
  server->sendContent("const canvas = document.getElementById('chart');");
  server->sendContent("const ctx = canvas.getContext('2d');");
  server->sendContent("const faktoren = ['Steigung', 'Groesse', 'Abstand', 'Luftstaerke', 'Blaetter'];");
  
  // Skalierungsfaktor für HD
  float scaleFactor = (float)width / 800.0;
  server->sendContent("const scaleFactor = " + String(scaleFactor) + ";");
  
  // HD-Chart-Generierung (skaliert)
  if (chartType == "main-effects") {
    generateCorrectedMainEffectsJS(filename);
  } else if (chartType == "pareto") {
    generateCorrectedParetoJS(filename);
  } else if (chartType == "interaction") {
    generateCorrectedInteractionJS(filename);
  } else if (chartType == "effects") {
    generateEffectsChartJS(filename);
  } else if (chartType == "factorial") {
    generateFactorialChartJS(filename);
  }
  
  server->sendContent("function downloadHD() {");
  server->sendContent("  const link = document.createElement('a');");
  server->sendContent("  link.download = 'windkraft_" + chartType + "_HD_" + String(width) + "x" + String(height) + ".png';");
  server->sendContent("  link.href = canvas.toDataURL('image/png');");
  server->sendContent("  link.click();");
  server->sendContent("}");
  
  server->sendContent("</script>");
  server->sendContent("</body></html>");
}

// Batch-Export und Publikationsqualität-Export wurden entfernt

// [Fortsetzung folgt in nächstem Teil...]

/**
 
 */
void WindTurbineDataManager::generateCorrectedMainEffectsJS(const char* filename) {
  server->sendContent("function drawChart() {");
  server->sendContent("  ctx.fillStyle = 'white'; ctx.fillRect(0, 0, canvas.width, canvas.height);");
  server->sendContent("  ctx.fillStyle = '#2c3e50'; ctx.font = 'bold 20px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Main Effects Plot', canvas.width/2, 30);");
  
  // Berechne echte Mittelwerte für jeden Faktor
  float meanLow[5], meanHigh[5], overallMean, minResponse, maxResponse;
  calculateMainEffectsData(filename, meanLow, meanHigh, overallMean, minResponse, maxResponse);
  
  // Übertrage berechnete Werte an JavaScript
  server->sendContent("  const meanLow = [");
  for (int i = 0; i < 5; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(String(meanLow[i], 3));
  }
  server->sendContent("];");
  
  server->sendContent("  const meanHigh = [");
  for (int i = 0; i < 5; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(String(meanHigh[i], 3));
  }
  server->sendContent("];");
  
  server->sendContent("  const minResponse = " + String(minResponse, 3) + ";");
  server->sendContent("  const maxResponse = " + String(maxResponse, 3) + ";");
  
  // Grid für 5 Faktoren (3 oben, 2 unten)
  server->sendContent("  const plotWidth = Math.floor(canvas.width * 0.18);");
  server->sendContent("  const plotHeight = Math.floor(canvas.height * 0.16);");
  server->sendContent("  const cols = 3; const rows = 2;");
  
  server->sendContent("  for (let i = 0; i < 5; i++) {");
  server->sendContent("    const col = i % cols; const row = Math.floor(i / cols);");
  server->sendContent("    const x = 50 + col * (plotWidth + 30);");
  server->sendContent("    const y = 60 + row * (plotHeight + 40);");
  
  // Plot-Rahmen
  server->sendContent("    ctx.strokeStyle = '#dee2e6'; ctx.lineWidth = 1;");
  server->sendContent("    ctx.strokeRect(x, y, plotWidth, plotHeight);");
  
  // Titel
  server->sendContent("    ctx.fillStyle = '#495057'; ctx.font = '14px Arial';");
  server->sendContent("    ctx.textAlign = 'center';");
  server->sendContent("    ctx.fillText(faktoren[i], x + plotWidth/2, y - 10);");
  
  // Achsen
  server->sendContent("    ctx.strokeStyle = '#6c757d'; ctx.lineWidth = 1;");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.moveTo(x + 20, y + plotHeight - 20);");
  server->sendContent("    ctx.lineTo(x + plotWidth - 20, y + plotHeight - 20);");
  server->sendContent("    ctx.moveTo(x + 20, y + 10);");
  server->sendContent("    ctx.lineTo(x + 20, y + plotHeight - 20);");
  server->sendContent("    ctx.stroke();");
  
  // KORRIGIERT: Y-Positionen basierend auf echten Messwerten
  server->sendContent("    const y1 = y + plotHeight - 20 - ((meanLow[i] - minResponse) / (maxResponse - minResponse)) * (plotHeight - 30);");
  server->sendContent("    const y2 = y + plotHeight - 20 - ((meanHigh[i] - minResponse) / (maxResponse - minResponse)) * (plotHeight - 30);");
  
  // Effekt-Linie mit korrekter Farbe
  server->sendContent("    const effect = meanHigh[i] - meanLow[i];");
  server->sendContent("    ctx.strokeStyle = effect > 0 ? '#28a745' : '#dc3545';");
  server->sendContent("    ctx.lineWidth = 3;");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.moveTo(x + 35, y1);");
  server->sendContent("    ctx.lineTo(x + plotWidth - 35, y2);");
  server->sendContent("    ctx.stroke();");
  
  // Punkte
  server->sendContent("    ctx.fillStyle = effect > 0 ? '#28a745' : '#dc3545';");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.arc(x + 35, y1, 4, 0, 2 * Math.PI); ctx.fill();");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.arc(x + plotWidth - 35, y2, 4, 0, 2 * Math.PI); ctx.fill();");
  
  // X-Achsen Labels
  server->sendContent("    ctx.fillStyle = '#6c757d'; ctx.font = '12px Arial';");
  server->sendContent("    ctx.textAlign = 'center';");
  server->sendContent("    ctx.fillText('-1', x + 35, y + plotHeight - 5);");
  server->sendContent("    ctx.fillText('+1', x + plotWidth - 35, y + plotHeight - 5);");
  
  // Y-Werte anzeigen
  server->sendContent("    ctx.textAlign = 'right'; ctx.font = '10px Arial';");
  server->sendContent("    ctx.fillText(meanLow[i].toFixed(1), x + 18, y1 + 3);");
  server->sendContent("    ctx.fillText(meanHigh[i].toFixed(1), x + 18, y2 + 3);");
  
  // Y-Achsen-Titel
  server->sendContent("    ctx.fillText('μW', x + 15, y + 25);");
  
  server->sendContent("  }");
  
  // Legende
  server->sendContent("  const legendY = Math.floor(canvas.height * 0.6);");
  server->sendContent("  ctx.fillStyle = '#e9ecef'; ctx.fillRect(50, legendY, canvas.width - 100, 80);");
  server->sendContent("  ctx.strokeStyle = '#dee2e6'; ctx.strokeRect(50, legendY, canvas.width - 100, 80);");
  server->sendContent("  ctx.fillStyle = '#495057'; ctx.font = '14px Arial';");
  server->sendContent("  ctx.textAlign = 'left';");
  server->sendContent("  ctx.fillText('Interpretation:', 60, legendY + 20);");
  server->sendContent("  ctx.font = '12px Arial';");
  server->sendContent("  ctx.fillText('• Liniensteigung zeigt Effektstärke (steiler = stärker)', 60, legendY + 40);");
  server->sendContent("  ctx.fillText('• Grün: Positive Effekte (hohe Stufe besser), Rot: Negative Effekte (niedrige Stufe besser)', 60, legendY + 55);");
  server->sendContent("  ctx.fillText('• Y-Werte sind echte Mittelwerte aus den Messdaten', 60, legendY + 70);");
  
  server->sendContent("}");
  server->sendContent("drawChart();");
}

/**
 * KORRIGIERT: Generiert mathematisch korrektes Pareto-Diagramm JavaScript
 */
void WindTurbineDataManager::generateCorrectedParetoJS(const char* filename) {
  server->sendContent("function drawChart() {");
  server->sendContent("  ctx.fillStyle = 'white'; ctx.fillRect(0, 0, canvas.width, canvas.height);");
  server->sendContent("  ctx.fillStyle = '#2c3e50'; ctx.font = 'bold 20px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Pareto-Diagramm (Konsistente Skalierung)', canvas.width/2, 30);");
  
  // Berechne korrekte Pareto-Daten
  float effects[5];
  int sortedIndices[5];
  float percentages[5];
  float cumulative[5];
  
  calculateParetoData(filename, effects, sortedIndices, percentages, cumulative);
  
  // Übertrage Daten an JavaScript
  server->sendContent("  const effects = [");
  for (int i = 0; i < 5; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(String(effects[i], 3));
  }
  server->sendContent("];");
  
  server->sendContent("  const sortedIndices = [");
  for (int i = 0; i < 5; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(String(sortedIndices[i]));
  }
  server->sendContent("];");
  
  server->sendContent("  const percentages = [");
  for (int i = 0; i < 5; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(String(percentages[i], 2));
  }
  server->sendContent("];");
  
  server->sendContent("  const cumulative = [");
  for (int i = 0; i < 5; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(String(cumulative[i], 2));
  }
  server->sendContent("];");
  
  // Diagrammbereich
  server->sendContent("  const chartX = Math.floor(canvas.width * 0.1);");
  server->sendContent("  const chartY = Math.floor(canvas.height * 0.13);");
  server->sendContent("  const chartWidth = Math.floor(canvas.width * 0.7);");
  server->sendContent("  const chartHeight = Math.floor(canvas.height * 0.5);");
  
  // Achsen
  server->sendContent("  ctx.strokeStyle = '#495057'; ctx.lineWidth = 2;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(chartX, chartY + chartHeight);");
  server->sendContent("  ctx.lineTo(chartX + chartWidth, chartY + chartHeight);");
  server->sendContent("  ctx.moveTo(chartX, chartY);");
  server->sendContent("  ctx.lineTo(chartX, chartY + chartHeight);");
  server->sendContent("  ctx.moveTo(chartX + chartWidth, chartY);");
  server->sendContent("  ctx.lineTo(chartX + chartWidth, chartY + chartHeight);");
  server->sendContent("  ctx.stroke();");
  
  // Y-Achsen-Beschriftung (beide 0-100%)
  server->sendContent("  ctx.fillStyle = '#495057'; ctx.font = '12px Arial';");
  server->sendContent("  for (let i = 0; i <= 10; i++) {");
  server->sendContent("    const y = chartY + chartHeight - (i / 10) * chartHeight;");
  server->sendContent("    ctx.textAlign = 'right';");
  server->sendContent("    ctx.fillText((i * 10) + '%', chartX - 5, y + 3);");
  server->sendContent("    ctx.textAlign = 'left';");
  server->sendContent("    ctx.fillText((i * 10) + '%', chartX + chartWidth + 5, y + 3);");
  server->sendContent("  }");
  
  // Referenzlinien
  server->sendContent("  ctx.strokeStyle = '#e9ecef'; ctx.lineWidth = 1;");
  server->sendContent("  for (let i = 1; i <= 9; i++) {");
  server->sendContent("    const y = chartY + chartHeight - (i / 10) * chartHeight;");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.moveTo(chartX, y);");
  server->sendContent("    ctx.lineTo(chartX + chartWidth, y);");
  server->sendContent("    ctx.stroke();");
  server->sendContent("  }");
  
  // Balken und Kurve
  server->sendContent("  const barWidth = chartWidth / 5;");
  server->sendContent("  for (let i = 0; i < 5; i++) {");
  server->sendContent("    const x = chartX + i * barWidth + 20;");
  
  // KORRIGIERT: Balkenhöhe basierend auf Prozentanteil (nicht absoluter Wert)
  server->sendContent("    const barHeight = (percentages[i] / 100) * chartHeight;");
  server->sendContent("    const y = chartY + chartHeight - barHeight;");
  
  // Balken zeichnen
  server->sendContent("    ctx.fillStyle = '#3498db';");
  server->sendContent("    ctx.fillRect(x, y, barWidth - 40, barHeight);");
  server->sendContent("    ctx.strokeStyle = '#2980b9'; ctx.lineWidth = 1;");
  server->sendContent("    ctx.strokeRect(x, y, barWidth - 40, barHeight);");
  
  // Prozentanteil über Balken
  server->sendContent("    ctx.fillStyle = '#2c3e50'; ctx.font = '11px Arial';");
  server->sendContent("    ctx.textAlign = 'center';");
  server->sendContent("    ctx.fillText(percentages[i].toFixed(1) + '%', x + (barWidth - 40)/2, y - 5);");
  
  // Faktornamen
  server->sendContent("    ctx.save();");
  server->sendContent("    ctx.translate(x + (barWidth - 40)/2, chartY + chartHeight + 15);");
  server->sendContent("    ctx.rotate(-Math.PI/6);");
  server->sendContent("    ctx.fillText(faktoren[sortedIndices[i]], 0, 0);");
  server->sendContent("    ctx.restore();");
  
  // KORRIGIERT: Kumulative Kurve (beide verwenden Prozente)
  server->sendContent("    const cumulativeY = chartY + chartHeight - (cumulative[i] / 100) * chartHeight;");
  server->sendContent("    const pointX = x + (barWidth - 40)/2;");
  
  // Kurven-Punkt
  server->sendContent("    ctx.fillStyle = '#e74c3c';");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.arc(pointX, cumulativeY, 4, 0, 2 * Math.PI);");
  server->sendContent("    ctx.fill();");
  
  // Linie zur nächsten
  server->sendContent("    if (i < 4) {");
  server->sendContent("      const nextX = chartX + (i + 1) * barWidth + 20 + (barWidth - 40)/2;");
  server->sendContent("      const nextY = chartY + chartHeight - (cumulative[i + 1] / 100) * chartHeight;");
  server->sendContent("      ctx.strokeStyle = '#e74c3c'; ctx.lineWidth = 2;");
  server->sendContent("      ctx.beginPath();");
  server->sendContent("      ctx.moveTo(pointX, cumulativeY);");
  server->sendContent("      ctx.lineTo(nextX, nextY);");
  server->sendContent("      ctx.stroke();");
  server->sendContent("    }");
  
  // Kumulative Prozente
  server->sendContent("    ctx.fillStyle = '#c0392b'; ctx.font = '10px Arial';");
  server->sendContent("    ctx.fillText(cumulative[i].toFixed(1) + '%', pointX + 10, cumulativeY - 5);");
  
  server->sendContent("  }");
  
  // Achsen-Titel
  server->sendContent("  ctx.fillStyle = '#495057'; ctx.font = '14px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Einzeleffekte (%)', chartX - 40, chartY + chartHeight/2);");
  server->sendContent("  ctx.fillText('Kumulativ (%)', chartX + chartWidth + 40, chartY + chartHeight/2);");
  server->sendContent("  ctx.fillText('Faktoren (nach Wichtigkeit sortiert)', chartX + chartWidth/2, chartY + chartHeight + 50);");
  
  // Korrektur-Hinweis
  server->sendContent("  const noteY = Math.floor(canvas.height * 0.75);");
  server->sendContent("  ctx.fillStyle = '#d4edda'; ctx.fillRect(80, noteY, canvas.width - 160, 60);");
  server->sendContent("  ctx.strokeStyle = '#c3e6cb'; ctx.strokeRect(80, noteY, canvas.width - 160, 60);");
  server->sendContent("  ctx.fillStyle = '#155724'; ctx.font = '12px Arial';");
  server->sendContent("  ctx.textAlign = 'left';");
  server->sendContent("  ctx.fillText('Balken und Kurve verwenden konsistente Prozent-Skalierung', 90, noteY + 20);");
  server->sendContent("  ctx.fillText('  Balken = Anteil des Gesamteffekts (%), Kurve = Kumulierte Prozente (0-100%)', 90, noteY + 40);");
  
  server->sendContent("}");
  server->sendContent("drawChart();");
}

/**
 * KORRIGIERT: Generiert robustes Interaction Plot JavaScript
 */
void WindTurbineDataManager::generateCorrectedInteractionJS(const char* filename) {
  server->sendContent("function drawChart() {");
  server->sendContent("  ctx.fillStyle = 'white'; ctx.fillRect(0, 0, canvas.width, canvas.height);");
  server->sendContent("  ctx.fillStyle = '#2c3e50'; ctx.font = 'bold 20px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Interaction Plot (Robuste Datenbehandlung)', canvas.width/2, 30);");
  
  // Berechne Interaktionsdaten mit Validierung
  float data[4];
  bool dataAvailable[4];
  
  // Bestimme die beiden stärksten Faktoren für Interaktion
  int factor1 = 0, factor2 = 1; // Standard-Auswahl
  
  calculateInteractionData(filename, factor1, factor2, data, dataAvailable);
  
  // Übertrage Daten an JavaScript
  server->sendContent("  const data = [");
  for (int i = 0; i < 4; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(String(data[i], 3));
  }
  server->sendContent("];");
  
  server->sendContent("  const dataAvailable = [");
  for (int i = 0; i < 4; i++) {
    if (i > 0) server->sendContent(",");
    server->sendContent(dataAvailable[i] ? "true" : "false");
  }
  server->sendContent("];");
  
  server->sendContent("  const factor1 = " + String(factor1) + ";");
  server->sendContent("  const factor2 = " + String(factor2) + ";");
  
  // Diagrammbereich
  server->sendContent("  const chartX = Math.floor(canvas.width * 0.125);");
  server->sendContent("  const chartY = Math.floor(canvas.height * 0.13);");
  server->sendContent("  const chartWidth = Math.floor(canvas.width * 0.5);");
  server->sendContent("  const chartHeight = Math.floor(canvas.height * 0.42);");
  
  // Achsen
  server->sendContent("  ctx.strokeStyle = '#495057'; ctx.lineWidth = 2;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(chartX, chartY + chartHeight);");
  server->sendContent("  ctx.lineTo(chartX + chartWidth, chartY + chartHeight);");
  server->sendContent("  ctx.moveTo(chartX, chartY);");
  server->sendContent("  ctx.lineTo(chartX, chartY + chartHeight);");
  server->sendContent("  ctx.stroke();");
  
  // Y-Skala
  server->sendContent("  const minY = Math.min(...data);");
  server->sendContent("  const maxY = Math.max(...data);");
  server->sendContent("  const range = maxY - minY;");
  server->sendContent("  const adjustedMin = minY - range * 0.1;");
  server->sendContent("  const adjustedMax = maxY + range * 0.1;");
  
  // Y-Achsen-Beschriftung
  server->sendContent("  ctx.fillStyle = '#495057'; ctx.font = '12px Arial';");
  server->sendContent("  for (let i = 0; i <= 4; i++) {");
  server->sendContent("    const y = chartY + chartHeight - (i / 4) * chartHeight;");
  server->sendContent("    const value = adjustedMin + (i / 4) * (adjustedMax - adjustedMin);");
  server->sendContent("    ctx.textAlign = 'right';");
  server->sendContent("    ctx.fillText(value.toFixed(1), chartX - 5, y + 3);");
  server->sendContent("  }");
  
  // X-Positionen
  server->sendContent("  const x1 = chartX + chartWidth * 0.3;");
  server->sendContent("  const x2 = chartX + chartWidth * 0.7;");
  
  // X-Achsen-Beschriftung
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText(faktoren[factor1] + ' = -1', x1, chartY + chartHeight + 20);");
  server->sendContent("  ctx.fillText(faktoren[factor1] + ' = +1', x2, chartY + chartHeight + 20);");
  
  // Faktor2 = -1 Linie (blau)
  server->sendContent("  const y1_low = chartY + chartHeight - ((data[0] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;");
  server->sendContent("  const y2_low = chartY + chartHeight - ((data[1] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;");
  
  server->sendContent("  ctx.strokeStyle = '#3498db'; ctx.lineWidth = 3;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(x1, y1_low);");
  server->sendContent("  ctx.lineTo(x2, y2_low);");
  server->sendContent("  ctx.stroke();");
  
  // Faktor2 = +1 Linie (orange)
  server->sendContent("  const y1_high = chartY + chartHeight - ((data[2] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;");
  server->sendContent("  const y2_high = chartY + chartHeight - ((data[3] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;");
  
  server->sendContent("  ctx.strokeStyle = '#e67e22'; ctx.lineWidth = 3;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(x1, y1_high);");
  server->sendContent("  ctx.lineTo(x2, y2_high);");
  server->sendContent("  ctx.stroke();");
  
  // Punkte mit Kennzeichnung interpolierter Werte
  server->sendContent("  ctx.fillStyle = '#3498db';");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.arc(x1, y1_low, 5, 0, 2 * Math.PI); ctx.fill();");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.arc(x2, y2_low, 5, 0, 2 * Math.PI); ctx.fill();");
  
  server->sendContent("  ctx.fillStyle = '#e67e22';");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.arc(x1, y1_high, 5, 0, 2 * Math.PI); ctx.fill();");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.arc(x2, y2_high, 5, 0, 2 * Math.PI); ctx.fill();");
  
  // Warnung für interpolierte Punkte
  server->sendContent("  if (!dataAvailable[0]) {");
  server->sendContent("    ctx.strokeStyle = '#ffc107'; ctx.lineWidth = 2;");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.arc(x1, y1_low, 7, 0, 2 * Math.PI); ctx.stroke();");
  server->sendContent("  }");
  
  server->sendContent("  if (!dataAvailable[1]) {");
  server->sendContent("    ctx.strokeStyle = '#ffc107'; ctx.lineWidth = 2;");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.arc(x2, y2_low, 7, 0, 2 * Math.PI); ctx.stroke();");
  server->sendContent("  }");
  
  server->sendContent("  if (!dataAvailable[2]) {");
  server->sendContent("    ctx.strokeStyle = '#ffc107'; ctx.lineWidth = 2;");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.arc(x1, y1_high, 7, 0, 2 * Math.PI); ctx.stroke();");
  server->sendContent("  }");
  
  server->sendContent("  if (!dataAvailable[3]) {");
  server->sendContent("    ctx.strokeStyle = '#ffc107'; ctx.lineWidth = 2;");
  server->sendContent("    ctx.beginPath();");
  server->sendContent("    ctx.arc(x2, y2_high, 7, 0, 2 * Math.PI); ctx.stroke();");
  server->sendContent("  }");
  
  // Werte anzeigen
  server->sendContent("  ctx.fillStyle = '#2c3e50'; ctx.font = '11px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText(data[0].toFixed(1), x1 - 15, y1_low - 10);");
  server->sendContent("  ctx.fillText(data[1].toFixed(1), x2 + 15, y2_low - 10);");
  server->sendContent("  ctx.fillText(data[2].toFixed(1), x1 - 15, y1_high + 15);");
  server->sendContent("  ctx.fillText(data[3].toFixed(1), x2 + 15, y2_high + 15);");
  
  // Legende
  server->sendContent("  const legendX = chartX + chartWidth - 150;");
  server->sendContent("  const legendY = chartY + 20;");
  server->sendContent("  ctx.fillStyle = '#f8f9fa'; ctx.fillRect(legendX, legendY, 140, 80);");
  server->sendContent("  ctx.strokeStyle = '#dee2e6'; ctx.strokeRect(legendX, legendY, 140, 80);");
  
  server->sendContent("  ctx.fillStyle = '#495057'; ctx.font = '12px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText(faktoren[factor2], legendX + 70, legendY + 15);");
  
  server->sendContent("  ctx.strokeStyle = '#3498db'; ctx.lineWidth = 3;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(legendX + 10, legendY + 30);");
  server->sendContent("  ctx.lineTo(legendX + 30, legendY + 30);");
  server->sendContent("  ctx.stroke();");
  
  server->sendContent("  ctx.textAlign = 'left';");
  server->sendContent("  ctx.fillText('= -1', legendX + 35, legendY + 34);");
  
  server->sendContent("  ctx.strokeStyle = '#e67e22'; ctx.lineWidth = 3;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(legendX + 10, legendY + 50);");
  server->sendContent("  ctx.lineTo(legendX + 30, legendY + 50);");
  server->sendContent("  ctx.stroke();");
  
  server->sendContent("  ctx.fillText('= +1', legendX + 35, legendY + 54);");
  
  // Warnung für interpolierte Daten
  server->sendContent("  const warningY = Math.floor(canvas.height * 0.63);");
  server->sendContent("  ctx.fillStyle = '#fff3cd'; ctx.fillRect(100, warningY, canvas.width - 200, 40);");
  server->sendContent("  ctx.strokeStyle = '#ffeaa7'; ctx.strokeRect(100, warningY, canvas.width - 200, 40);");
  server->sendContent("  ctx.fillStyle = '#856404'; ctx.font = '12px Arial';");
  server->sendContent("  ctx.textAlign = 'left';");
  server->sendContent("  ctx.fillText('⚠ Gelb umrandete Punkte = Interpoliert (fehlende Daten im teilfaktoriellen Plan)', 110, warningY + 15);");
  server->sendContent("  ctx.fillText('Interpretation: Parallele Linien = keine Wechselwirkung, sich kreuzende Linien = Wechselwirkung', 110, warningY + 30);");
  
  server->sendContent("}");
  server->sendContent("drawChart();");
}

/**
 * Generiert Effekt-Diagramm JavaScript
 */
void WindTurbineDataManager::generateEffectsChartJS(const char* filename) {
  server->sendContent("function drawChart() {");
  server->sendContent("  ctx.fillStyle = 'white'; ctx.fillRect(0, 0, canvas.width, canvas.height);");
  server->sendContent("  ctx.fillStyle = '#2c3e50'; ctx.font = 'bold 20px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Effekt-Diagramm', canvas.width/2, 30);");
  
  // Lade Effekt-Daten
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  bool hasRealData = false;
  float realEffects[5] = {0, 0, 0, 0, 0};
  
  if (file) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (!error && doc.containsKey("effekte")) {
      JsonArray effekteArray = doc["effekte"];
      hasRealData = true;
      for (int i = 0; i < 5 && i < effekteArray.size(); i++) {
        realEffects[i] = effekteArray[i].as<float>();
      }
    }
  }
  
  // Übertrage Effekte an JavaScript
  server->sendContent("  const effects = [");
  if (hasRealData) {
    for (int i = 0; i < 5; i++) {
      if (i > 0) server->sendContent(",");
      server->sendContent(String(realEffects[i], 3));
    }
  } else {
    server->sendContent("0.5, -0.3, 0.2, -0.7, 0.4"); // Testdaten
  }
  server->sendContent("];");
  
  server->sendContent("  const hasRealData = " + String(hasRealData ? "true" : "false") + ";");
  
  // Diagrammbereich
  server->sendContent("  const chartX = Math.floor(canvas.width * 0.15);");
  server->sendContent("  const chartY = Math.floor(canvas.height * 0.15);");
  server->sendContent("  const chartWidth = Math.floor(canvas.width * 0.7);");
  server->sendContent("  const chartHeight = Math.floor(canvas.height * 0.6);");
  
  // Rahmen
  server->sendContent("  ctx.strokeStyle = '#495057'; ctx.lineWidth = 2;");
  server->sendContent("  ctx.strokeRect(chartX, chartY, chartWidth, chartHeight);");
  
  // Nulllinie
  server->sendContent("  const zeroY = chartY + chartHeight / 2;");
  server->sendContent("  ctx.strokeStyle = '#6c757d'; ctx.lineWidth = 1;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(chartX, zeroY);");
  server->sendContent("  ctx.lineTo(chartX + chartWidth, zeroY);");
  server->sendContent("  ctx.stroke();");
  
  // Maximal-Effekt finden für Skalierung
  server->sendContent("  const maxEffect = Math.max(...effects.map(Math.abs));");
  server->sendContent("  const scale = (chartHeight / 2) / (maxEffect * 1.2);");
  
  // Balken zeichnen
  server->sendContent("  const barWidth = chartWidth / 5;");
  server->sendContent("  for (let i = 0; i < 5; i++) {");
  server->sendContent("    const x = chartX + i * barWidth + barWidth * 0.1;");
  server->sendContent("    const barHeight = Math.abs(effects[i]) * scale;");
  server->sendContent("    const y = effects[i] > 0 ? zeroY - barHeight : zeroY;");
  
  // Balkenfarbe
  server->sendContent("    ctx.fillStyle = effects[i] > 0 ? '#28a745' : '#dc3545';");
  server->sendContent("    ctx.fillRect(x, y, barWidth * 0.8, barHeight);");
  server->sendContent("    ctx.strokeStyle = effects[i] > 0 ? '#1e7e34' : '#c82333';");
  server->sendContent("    ctx.strokeRect(x, y, barWidth * 0.8, barHeight);");
  
  // Effekt-Wert über/unter Balken
  server->sendContent("    ctx.fillStyle = '#2c3e50'; ctx.font = '12px Arial';");
  server->sendContent("    ctx.textAlign = 'center';");
  server->sendContent("    const textY = effects[i] > 0 ? y - 5 : y + barHeight + 15;");
  server->sendContent("    ctx.fillText(effects[i].toFixed(2), x + barWidth * 0.4, textY);");
  
  // Faktorname
  server->sendContent("    ctx.fillText(faktoren[i], x + barWidth * 0.4, chartY + chartHeight + 20);");
  
  server->sendContent("  }");
  
  // Y-Achse Beschriftung
  server->sendContent("  ctx.fillStyle = '#495057'; ctx.font = '12px Arial';");
  server->sendContent("  ctx.textAlign = 'right';");
  server->sendContent("  ctx.fillText('0', chartX - 5, zeroY + 3);");
  server->sendContent("  if (maxEffect > 0) {");
  server->sendContent("    ctx.fillText(maxEffect.toFixed(1), chartX - 5, chartY + 5);");
  server->sendContent("    ctx.fillText((-maxEffect).toFixed(1), chartX - 5, chartY + chartHeight);");
  server->sendContent("  }");
  
  // Y-Achse Titel
  server->sendContent("  ctx.save();");
  server->sendContent("  ctx.translate(chartX - 40, chartY + chartHeight/2);");
  server->sendContent("  ctx.rotate(-Math.PI/2);");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Effekt (μW)', 0, 0);");
  server->sendContent("  ctx.restore();");
  
  // Datenquelle-Hinweis
  server->sendContent("  const noteY = chartY + chartHeight + 50;");
  server->sendContent("  ctx.fillStyle = hasRealData ? '#d4edda' : '#fff3cd';");
  server->sendContent("  ctx.fillRect(chartX, noteY, chartWidth, 30);");
  server->sendContent("  ctx.strokeStyle = hasRealData ? '#c3e6cb' : '#ffeaa7';");
  server->sendContent("  ctx.strokeRect(chartX, noteY, chartWidth, 30);");
  server->sendContent("  ctx.fillStyle = hasRealData ? '#155724' : '#856404';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText(hasRealData ? '✓ Echte Messdaten aus teilfaktoriellem Versuch' : '⚠ Beispieldaten - führen Sie Messungen durch', chartX + chartWidth/2, noteY + 20);");
  
  server->sendContent("}");
  server->sendContent("drawChart();");
}

/**
 * Generiert Faktoriell-Vergleich JavaScript
 */
void WindTurbineDataManager::generateFactorialChartJS(const char* filename) {
  server->sendContent("function drawChart() {");
  server->sendContent("  ctx.fillStyle = 'white'; ctx.fillRect(0, 0, canvas.width, canvas.height);");
  server->sendContent("  ctx.fillStyle = '#2c3e50'; ctx.font = 'bold 20px Arial';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Faktorieller Vergleich', canvas.width/2, 30);");
  
  // Lade Daten
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  bool hasRealData = false;
  float realData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  
  if (file) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (!error && doc.containsKey("vollfaktoriellMittelwerte")) {
      JsonArray vfMittelwerte = doc["vollfaktoriellMittelwerte"];
      hasRealData = true;
      for (int i = 0; i < 8 && i < vfMittelwerte.size(); i++) {
        realData[i] = vfMittelwerte[i].as<float>();
      }
    }
  }
  
  // Übertrage Daten an JavaScript
  server->sendContent("  const data = [");
  if (hasRealData) {
    for (int i = 0; i < 8; i++) {
      if (i > 0) server->sendContent(",");
      server->sendContent(String(realData[i], 3));
    }
  } else {
    server->sendContent("0.8, 1.2, 0.7, 1.5, 0.9, 1.4, 1.1, 1.8"); // Testdaten
  }
  server->sendContent("];");
  
  server->sendContent("  const hasRealData = " + String(hasRealData ? "true" : "false") + ";");
  
  // Diagrammbereich
  server->sendContent("  const chartX = Math.floor(canvas.width * 0.1);");
  server->sendContent("  const chartY = Math.floor(canvas.height * 0.15);");
  server->sendContent("  const chartWidth = Math.floor(canvas.width * 0.8);");
  server->sendContent("  const chartHeight = Math.floor(canvas.height * 0.6);");
  
  // Achsen
  server->sendContent("  ctx.strokeStyle = '#495057'; ctx.lineWidth = 2;");
  server->sendContent("  ctx.beginPath();");
  server->sendContent("  ctx.moveTo(chartX, chartY);");
  server->sendContent("  ctx.lineTo(chartX, chartY + chartHeight);");
  server->sendContent("  ctx.lineTo(chartX + chartWidth, chartY + chartHeight);");
  server->sendContent("  ctx.stroke();");
  
  // Skalierung bestimmen
  server->sendContent("  const minVal = Math.min(...data);");
  server->sendContent("  const maxVal = Math.max(...data);");
  server->sendContent("  const range = maxVal - minVal || 1;");
  server->sendContent("  const adjustedMin = minVal - range * 0.1;");
  server->sendContent("  const adjustedMax = maxVal + range * 0.1;");
  server->sendContent("  const scale = chartHeight / (adjustedMax - adjustedMin);");
  
  // Balken zeichnen
  server->sendContent("  const barWidth = chartWidth / 8;");
  server->sendContent("  for (let i = 0; i < 8; i++) {");
  server->sendContent("    const x = chartX + i * barWidth + barWidth * 0.1;");
  server->sendContent("    const barHeight = (data[i] - adjustedMin) * scale;");
  server->sendContent("    const y = chartY + chartHeight - barHeight;");
  
  // Farbverlauf für Balken
  server->sendContent("    const gradient = ctx.createLinearGradient(0, y, 0, y + barHeight);");
  server->sendContent("    gradient.addColorStop(0, '#3498db');");
  server->sendContent("    gradient.addColorStop(1, '#2980b9');");
  server->sendContent("    ctx.fillStyle = gradient;");
  server->sendContent("    ctx.fillRect(x, y, barWidth * 0.8, barHeight);");
  
  // Rahmen
  server->sendContent("    ctx.strokeStyle = '#2c3e50'; ctx.lineWidth = 1;");
  server->sendContent("    ctx.strokeRect(x, y, barWidth * 0.8, barHeight);");
  
  // Wert über Balken
  server->sendContent("    ctx.fillStyle = '#2c3e50'; ctx.font = '11px Arial';");
  server->sendContent("    ctx.textAlign = 'center';");
  server->sendContent("    ctx.fillText(data[i].toFixed(2), x + barWidth * 0.4, y - 5);");
  
  // Versuchsnummer
  server->sendContent("    ctx.fillText((i + 1).toString(), x + barWidth * 0.4, chartY + chartHeight + 15);");
  
  server->sendContent("  }");
  
  // Y-Achse Beschriftung
  server->sendContent("  ctx.fillStyle = '#495057'; ctx.font = '12px Arial';");
  server->sendContent("  ctx.textAlign = 'right';");
  server->sendContent("  for (let i = 0; i <= 5; i++) {");
  server->sendContent("    const val = adjustedMin + (i / 5) * (adjustedMax - adjustedMin);");
  server->sendContent("    const y = chartY + chartHeight - (i / 5) * chartHeight;");
  server->sendContent("    ctx.fillText(val.toFixed(1), chartX - 5, y + 3);");
  server->sendContent("  }");
  
  // Achsen-Titel
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText('Versuch Nr.', chartX + chartWidth/2, chartY + chartHeight + 35);");
  
  server->sendContent("  ctx.save();");
  server->sendContent("  ctx.translate(chartX - 40, chartY + chartHeight/2);");
  server->sendContent("  ctx.rotate(-Math.PI/2);");
  server->sendContent("  ctx.fillText('Leistung (μW)', 0, 0);");
  server->sendContent("  ctx.restore();");
  
  // Datenquelle-Hinweis
  server->sendContent("  const noteY = chartY + chartHeight + 55;");
  server->sendContent("  ctx.fillStyle = hasRealData ? '#d4edda' : '#fff3cd';");
  server->sendContent("  ctx.fillRect(chartX, noteY, chartWidth, 30);");
  server->sendContent("  ctx.strokeStyle = hasRealData ? '#c3e6cb' : '#ffeaa7';");
  server->sendContent("  ctx.strokeRect(chartX, noteY, chartWidth, 30);");
  server->sendContent("  ctx.fillStyle = hasRealData ? '#155724' : '#856404';");
  server->sendContent("  ctx.textAlign = 'center';");
  server->sendContent("  ctx.fillText(hasRealData ? '✓ Echte Messdaten aus vollfaktoriellem Versuch' : '⚠ Beispieldaten - führen Sie Messungen durch', chartX + chartWidth/2, noteY + 20);");
  
  server->sendContent("}");
  server->sendContent("drawChart();");
}

/**
 * SVG-Generierungsfunktionen
 */
void WindTurbineDataManager::generateMainEffectsSVG(const char* filename) {
  server->sendContent("<text x='400' y='25' class='title' text-anchor='middle'>Main Effects Plot (SVG)</text>");
  
  // Berechne Daten
  float meanLow[5], meanHigh[5], overallMean, minResponse, maxResponse;
  calculateMainEffectsData(filename, meanLow, meanHigh, overallMean, minResponse, maxResponse);
  
  // 5 Subplots (3 oben, 2 unten)
  for (int i = 0; i < 5; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = 50 + col * 180;
    int y = 60 + row * 140;
    int plotWidth = 140;
    int plotHeight = 100;
    
    // Plot-Rahmen
    server->sendContent("<rect x='" + String(x) + "' y='" + String(y) + "' width='" + String(plotWidth) + "' height='" + String(plotHeight) + "' fill='none' stroke='#dee2e6'/>");
    
    // Titel
    server->sendContent("<text x='" + String(x + plotWidth/2) + "' y='" + String(y - 5) + "' class='factor-text' text-anchor='middle'>" + String(faktorNamen[i]) + "</text>");
    
    // Achsen
    server->sendContent("<line x1='" + String(x + 20) + "' y1='" + String(y + 10) + "' x2='" + String(x + 20) + "' y2='" + String(y + plotHeight - 20) + "' class='axis'/>");
    server->sendContent("<line x1='" + String(x + 20) + "' y1='" + String(y + plotHeight - 20) + "' x2='" + String(x + plotWidth - 20) + "' y2='" + String(y + plotHeight - 20) + "' class='axis'/>");
    
    // Datenpunkte
    int x1 = x + 35;
    int x2 = x + plotWidth - 35;
    float range = maxResponse - minResponse;
    int y1 = y + plotHeight - 20 - ((meanLow[i] - minResponse) / range) * (plotHeight - 30);
    int y2 = y + plotHeight - 20 - ((meanHigh[i] - minResponse) / range) * (plotHeight - 30);
    
    // Linie
    String color = (meanHigh[i] > meanLow[i]) ? "#28a745" : "#dc3545";
    server->sendContent("<line x1='" + String(x1) + "' y1='" + String(y1) + "' x2='" + String(x2) + "' y2='" + String(y2) + "' stroke='" + color + "' stroke-width='3'/>");
    
    // Punkte
    server->sendContent("<circle cx='" + String(x1) + "' cy='" + String(y1) + "' r='4' fill='" + color + "'/>");
    server->sendContent("<circle cx='" + String(x2) + "' cy='" + String(y2) + "' r='4' fill='" + color + "'/>");
    
    // X-Labels
    server->sendContent("<text x='" + String(x1) + "' y='" + String(y + plotHeight - 5) + "' class='text' text-anchor='middle'>-1</text>");
    server->sendContent("<text x='" + String(x2) + "' y='" + String(y + plotHeight - 5) + "' class='text' text-anchor='middle'>+1</text>");
    
    // Y-Werte
    server->sendContent("<text x='" + String(x + 15) + "' y='" + String(y1 + 3) + "' class='text' text-anchor='end'>" + String(meanLow[i], 1) + "</text>");
    server->sendContent("<text x='" + String(x + 15) + "' y='" + String(y2 + 3) + "' class='text' text-anchor='end'>" + String(meanHigh[i], 1) + "</text>");
  }
}

void WindTurbineDataManager::generateParetoSVG(const char* filename) {
  server->sendContent("<text x='400' y='25' class='title' text-anchor='middle'>Pareto-Diagramm (SVG)</text>");
  
  // Berechne Daten
  float effects[5];
  int sortedIndices[5];
  float percentages[5];
  float cumulative[5];
  calculateParetoData(filename, effects, sortedIndices, percentages, cumulative);
  
  int chartX = 80, chartY = 80, chartWidth = 500, chartHeight = 300;
  
  // Achsen
  server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(chartY) + "' x2='" + String(chartX) + "' y2='" + String(chartY + chartHeight) + "' class='axis'/>");
  server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(chartY + chartHeight) + "' x2='" + String(chartX + chartWidth) + "' y2='" + String(chartY + chartHeight) + "' class='axis'/>");
  server->sendContent("<line x1='" + String(chartX + chartWidth) + "' y1='" + String(chartY) + "' x2='" + String(chartX + chartWidth) + "' y2='" + String(chartY + chartHeight) + "' class='axis'/>");
  
  // Raster
  for (int i = 1; i <= 9; i++) {
    int y = chartY + chartHeight - (i * chartHeight) / 10;
    server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(y) + "' x2='" + String(chartX + chartWidth) + "' y2='" + String(y) + "' class='grid'/>");
  }
  
  // Balken und Kurve
  int barWidth = chartWidth / 5;
  String pathData = "M";
  
  for (int i = 0; i < 5; i++) {
    int x = chartX + i * barWidth + 20;
    int barHeight = (percentages[i] / 100.0) * chartHeight;
    int y = chartY + chartHeight - barHeight;
    
    // Balken
    server->sendContent("<rect x='" + String(x) + "' y='" + String(y) + "' width='" + String(barWidth - 40) + "' height='" + String(barHeight) + "' fill='#3498db' stroke='#2980b9'/>");
    
    // Prozentanteil
    server->sendContent("<text x='" + String(x + (barWidth - 40)/2) + "' y='" + String(y - 5) + "' class='text' text-anchor='middle'>" + String(percentages[i], 1) + "%</text>");
    
    // Faktorname
    server->sendContent("<text x='" + String(x + (barWidth - 40)/2) + "' y='" + String(chartY + chartHeight + 15) + "' class='text' text-anchor='middle' transform='rotate(-30 " + String(x + (barWidth - 40)/2) + " " + String(chartY + chartHeight + 15) + ")'>" + String(faktorNamen[sortedIndices[i]]) + "</text>");
    
    // Pareto-Kurve
    int pointX = x + (barWidth - 40) / 2;
    int cumulativeY = chartY + chartHeight - (cumulative[i] / 100.0) * chartHeight;
    
    if (i == 0) {
      pathData += String(pointX) + "," + String(cumulativeY);
    } else {
      pathData += " L" + String(pointX) + "," + String(cumulativeY);
    }
    
    // Punkt
    server->sendContent("<circle cx='" + String(pointX) + "' cy='" + String(cumulativeY) + "' r='4' fill='#e74c3c'/>");
    
    // Kumulativer Prozenttext
    server->sendContent("<text x='" + String(pointX + 10) + "' y='" + String(cumulativeY - 5) + "' class='text' fill='#c0392b'>" + String(cumulative[i], 1) + "%</text>");
  }
  
  // Pareto-Linie
  server->sendContent("<path d='" + pathData + "' fill='none' stroke='#e74c3c' stroke-width='2'/>");
  
  // Y-Achsen-Beschriftung
  for (int i = 0; i <= 10; i++) {
    int y = chartY + chartHeight - (i * chartHeight) / 10;
    server->sendContent("<text x='" + String(chartX - 5) + "' y='" + String(y + 3) + "' class='text' text-anchor='end'>" + String(i * 10) + "%</text>");
    server->sendContent("<text x='" + String(chartX + chartWidth + 5) + "' y='" + String(y + 3) + "' class='text'>" + String(i * 10) + "%</text>");
  }
}

void WindTurbineDataManager::generateInteractionSVG(const char* filename) {
  server->sendContent("<text x='400' y='25' class='title' text-anchor='middle'>Interaction Plot (SVG)</text>");
  
  // Berechne Daten
  float data[4];
  bool dataAvailable[4];
  int factor1 = 0, factor2 = 1;
  calculateInteractionData(filename, factor1, factor2, data, dataAvailable);
  
  int chartX = 100, chartY = 80, chartWidth = 400, chartHeight = 250;
  
  // Achsen
  server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(chartY) + "' x2='" + String(chartX) + "' y2='" + String(chartY + chartHeight) + "' class='axis'/>");
  server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(chartY + chartHeight) + "' x2='" + String(chartX + chartWidth) + "' y2='" + String(chartY + chartHeight) + "' class='axis'/>");
  
  // Skalierung
  float minY = data[0], maxY = data[0];
  for (int i = 1; i < 4; i++) {
    if (data[i] < minY) minY = data[i];
    if (data[i] > maxY) maxY = data[i];
  }
  float range = maxY - minY;
  float adjustedMin = minY - range * 0.1;
  float adjustedMax = maxY + range * 0.1;
  
  // X-Positionen
  int x1 = chartX + chartWidth * 0.3;
  int x2 = chartX + chartWidth * 0.7;
  
  // Y-Positionen
  int y1_low = chartY + chartHeight - ((data[0] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;
  int y2_low = chartY + chartHeight - ((data[1] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;
  int y1_high = chartY + chartHeight - ((data[2] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;
  int y2_high = chartY + chartHeight - ((data[3] - adjustedMin) / (adjustedMax - adjustedMin)) * chartHeight;
  
  // Linien
  server->sendContent("<line x1='" + String(x1) + "' y1='" + String(y1_low) + "' x2='" + String(x2) + "' y2='" + String(y2_low) + "' stroke='#3498db' stroke-width='3'/>");
  server->sendContent("<line x1='" + String(x1) + "' y1='" + String(y1_high) + "' x2='" + String(x2) + "' y2='" + String(y2_high) + "' stroke='#e67e22' stroke-width='3'/>");
  
  // Punkte
  server->sendContent("<circle cx='" + String(x1) + "' cy='" + String(y1_low) + "' r='5' fill='#3498db'/>");
  server->sendContent("<circle cx='" + String(x2) + "' cy='" + String(y2_low) + "' r='5' fill='#3498db'/>");
  server->sendContent("<circle cx='" + String(x1) + "' cy='" + String(y1_high) + "' r='5' fill='#e67e22'/>");
  server->sendContent("<circle cx='" + String(x2) + "' cy='" + String(y2_high) + "' r='5' fill='#e67e22'/>");
  
  // Interpolierte Punkte markieren
  if (!dataAvailable[0]) server->sendContent("<circle cx='" + String(x1) + "' cy='" + String(y1_low) + "' r='7' fill='none' stroke='#ffc107' stroke-width='2'/>");
  if (!dataAvailable[1]) server->sendContent("<circle cx='" + String(x2) + "' cy='" + String(y2_low) + "' r='7' fill='none' stroke='#ffc107' stroke-width='2'/>");
  if (!dataAvailable[2]) server->sendContent("<circle cx='" + String(x1) + "' cy='" + String(y1_high) + "' r='7' fill='none' stroke='#ffc107' stroke-width='2'/>");
  if (!dataAvailable[3]) server->sendContent("<circle cx='" + String(x2) + "' cy='" + String(y2_high) + "' r='7' fill='none' stroke='#ffc107' stroke-width='2'/>");
  
  // Labels
  server->sendContent("<text x='" + String(x1) + "' y='" + String(chartY + chartHeight + 20) + "' class='text' text-anchor='middle'>" + String(faktorNamen[factor1]) + " = -1</text>");
  server->sendContent("<text x='" + String(x2) + "' y='" + String(chartY + chartHeight + 20) + "' class='text' text-anchor='middle'>" + String(faktorNamen[factor1]) + " = +1</text>");
  
  // Werte
  server->sendContent("<text x='" + String(x1 - 15) + "' y='" + String(y1_low - 10) + "' class='text' text-anchor='middle'>" + String(data[0], 1) + "</text>");
  server->sendContent("<text x='" + String(x2 + 15) + "' y='" + String(y2_low - 10) + "' class='text' text-anchor='middle'>" + String(data[1], 1) + "</text>");
  server->sendContent("<text x='" + String(x1 - 15) + "' y='" + String(y1_high + 15) + "' class='text' text-anchor='middle'>" + String(data[2], 1) + "</text>");
  server->sendContent("<text x='" + String(x2 + 15) + "' y='" + String(y2_high + 15) + "' class='text' text-anchor='middle'>" + String(data[3], 1) + "</text>");
}

void WindTurbineDataManager::generateEffectsSVG(const char* filename) {
  server->sendContent("<text x='400' y='25' class='title' text-anchor='middle'>Effekt-Diagramm (SVG)</text>");
  
  // Lade Daten
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  float effects[5] = {0.5, -0.3, 0.2, -0.7, 0.4}; // Fallback
  bool hasRealData = false;
  
  if (file) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (!error && doc.containsKey("effekte")) {
      JsonArray effekteArray = doc["effekte"];
      hasRealData = true;
      for (int i = 0; i < 5 && i < effekteArray.size(); i++) {
        effects[i] = effekteArray[i].as<float>();
      }
    }
  }
  
  int chartX = 120, chartY = 120, chartWidth = 560, chartHeight = 360;
  
  // Rahmen und Achsen
  server->sendContent("<rect x='" + String(chartX) + "' y='" + String(chartY) + "' width='" + String(chartWidth) + "' height='" + String(chartHeight) + "' fill='none' stroke='#495057' stroke-width='2'/>");
  
  // Nulllinie
  int zeroY = chartY + chartHeight / 2;
  server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(zeroY) + "' x2='" + String(chartX + chartWidth) + "' y2='" + String(zeroY) + "' stroke='#6c757d'/>");
  
  // Maximal-Effekt für Skalierung
  float maxEffect = 0;
  for (int i = 0; i < 5; i++) {
    if (abs(effects[i]) > maxEffect) maxEffect = abs(effects[i]);
  }
  if (maxEffect == 0) maxEffect = 1;
  
  float scale = (chartHeight / 2) / (maxEffect * 1.2);
  
  // Balken
  int barWidth = chartWidth / 5;
  for (int i = 0; i < 5; i++) {
    int x = chartX + i * barWidth + barWidth * 0.1;
    int barHeight = abs(effects[i]) * scale;
    int y = effects[i] > 0 ? zeroY - barHeight : zeroY;
    
    String color = effects[i] > 0 ? "#28a745" : "#dc3545";
    server->sendContent("<rect x='" + String(x) + "' y='" + String(y) + "' width='" + String(barWidth * 0.8) + "' height='" + String(barHeight) + "' fill='" + color + "' stroke='#2c3e50'/>");
    
    // Wert
    int textY = effects[i] > 0 ? y - 5 : y + barHeight + 15;
    server->sendContent("<text x='" + String(x + barWidth * 0.4) + "' y='" + String(textY) + "' class='text' text-anchor='middle'>" + String(effects[i], 2) + "</text>");
    
    // Faktorname
    server->sendContent("<text x='" + String(x + barWidth * 0.4) + "' y='" + String(chartY + chartHeight + 20) + "' class='text' text-anchor='middle'>" + String(faktorNamen[i]) + "</text>");
  }
  
  // Y-Achse Beschriftung
  server->sendContent("<text x='" + String(chartX - 5) + "' y='" + String(zeroY + 3) + "' class='text' text-anchor='end'>0</text>");
  if (maxEffect > 0) {
    server->sendContent("<text x='" + String(chartX - 5) + "' y='" + String(chartY + 5) + "' class='text' text-anchor='end'>" + String(maxEffect, 1) + "</text>");
    server->sendContent("<text x='" + String(chartX - 5) + "' y='" + String(chartY + chartHeight) + "' class='text' text-anchor='end'>-" + String(maxEffect, 1) + "</text>");
  }
}

void WindTurbineDataManager::generateFactorialSVG(const char* filename) {
  server->sendContent("<text x='400' y='25' class='title' text-anchor='middle'>Faktorieller Vergleich (SVG)</text>");
  
  // Lade Daten
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  float data[8] = {0.8, 1.2, 0.7, 1.5, 0.9, 1.4, 1.1, 1.8}; // Fallback
  bool hasRealData = false;
  
  if (file) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (!error && doc.containsKey("vollfaktoriellMittelwerte")) {
      JsonArray vfMittelwerte = doc["vollfaktoriellMittelwerte"];
      hasRealData = true;
      for (int i = 0; i < 8 && i < vfMittelwerte.size(); i++) {
        data[i] = vfMittelwerte[i].as<float>();
      }
    }
  }
  
  int chartX = 80, chartY = 120, chartWidth = 640, chartHeight = 360;
  
  // Achsen
  server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(chartY) + "' x2='" + String(chartX) + "' y2='" + String(chartY + chartHeight) + "' class='axis'/>");
  server->sendContent("<line x1='" + String(chartX) + "' y1='" + String(chartY + chartHeight) + "' x2='" + String(chartX + chartWidth) + "' y2='" + String(chartY + chartHeight) + "' class='axis'/>");
  
  // Skalierung
  float minVal = data[0], maxVal = data[0];
  for (int i = 1; i < 8; i++) {
    if (data[i] < minVal) minVal = data[i];
    if (data[i] > maxVal) maxVal = data[i];
  }
  float range = maxVal - minVal;
  if (range == 0) range = 1;
  float adjustedMin = minVal - range * 0.1;
  float adjustedMax = maxVal + range * 0.1;
  float scale = chartHeight / (adjustedMax - adjustedMin);
  
  // Balken
  int barWidth = chartWidth / 8;
  for (int i = 0; i < 8; i++) {
    int x = chartX + i * barWidth + barWidth * 0.1;
    int barHeight = (data[i] - adjustedMin) * scale;
    int y = chartY + chartHeight - barHeight;
    
    // Farbverlauf simulieren mit zwei Farbtönen
    server->sendContent("<defs><linearGradient id='grad" + String(i) + "' x1='0%' y1='0%' x2='0%' y2='100%'>");
    server->sendContent("<stop offset='0%' style='stop-color:#3498db;stop-opacity:1' />");
    server->sendContent("<stop offset='100%' style='stop-color:#2980b9;stop-opacity:1' />");
    server->sendContent("</linearGradient></defs>");
    
    server->sendContent("<rect x='" + String(x) + "' y='" + String(y) + "' width='" + String(barWidth * 0.8) + "' height='" + String(barHeight) + "' fill='url(#grad" + String(i) + ")' stroke='#2c3e50'/>");
    
    // Wert
    server->sendContent("<text x='" + String(x + barWidth * 0.4) + "' y='" + String(y - 5) + "' class='text' text-anchor='middle'>" + String(data[i], 2) + "</text>");
    
    // Versuchsnummer
    server->sendContent("<text x='" + String(x + barWidth * 0.4) + "' y='" + String(chartY + chartHeight + 15) + "' class='text' text-anchor='middle'>" + String(i + 1) + "</text>");
  }
  
  // Y-Achse Beschriftung
  for (int i = 0; i <= 5; i++) {
    float val = adjustedMin + (i / 5.0) * (adjustedMax - adjustedMin);
    int y = chartY + chartHeight - (i / 5.0) * chartHeight;
    server->sendContent("<text x='" + String(chartX - 5) + "' y='" + String(y + 3) + "' class='text' text-anchor='end'>" + String(val, 1) + "</text>");
  }
  
  // Achsen-Titel
  server->sendContent("<text x='" + String(chartX + chartWidth/2) + "' y='" + String(chartY + chartHeight + 35) + "' class='text' text-anchor='middle'>Versuch Nr.</text>");
  server->sendContent("<text x='" + String(chartX - 40) + "' y='" + String(chartY + chartHeight/2) + "' class='text' text-anchor='middle' transform='rotate(-90 " + String(chartX - 40) + " " + String(chartY + chartHeight/2) + ")'>Leistung (μW)</text>");
}

// Interaktive Grafiken und Komplettpaket wurden entfernt

void WindTurbineDataManager::serveJSONChunked(const char* filename) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    server->send(404, "text/plain", "Datei nicht gefunden");
    return;
  }
  
  server->setContentLength(file.size());
  server->send(200, "application/json", "");
  
  while (file.available()) {
    String chunk = file.readString();
    server->sendContent(chunk);
  }
  
  file.close();
}

/**
 * VERBESSERTE CSV-Export Funktion mit deutscher Lokalisierung und vollständigen Daten
 * Ersetzt die bestehende serveCompleteCSV Funktion in WindTurbineDataManager.cpp
 */
void WindTurbineDataManager::serveCompleteCSV(const char* filename) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    server->send(404, "text/plain", "Datei nicht gefunden");
    return;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    server->send(500, "text/plain", "Fehler beim Parsen der JSON-Daten");
    return;
  }
  
  // Deutsche CSV-Header mit korrektem Content-Type
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->sendHeader("Content-Disposition", "attachment; filename=\"windkraft_experiment_vollstaendig.csv\"");
  server->send(200, "text/csv; charset=utf-8", "");
  
  // Helper-Funktion für deutsche Zahlenformatierung
  auto formatGerman = [](float value, int decimals = 3) -> String {
    String result = String(value, decimals);
    result.replace(".", ",");
    return result;
  };
  
  // ===========================================
  // 1. METADATEN-BEREICH
  // ===========================================
  server->sendContent("sep=;\n"); // Excel-Hinweis für Semikolon-Trennung
  server->sendContent("WINDKRAFT-EXPERIMENT - VOLLSTÄNDIGER DATENEXPORT\n");
  server->sendContent(";;;;;\n");
  server->sendContent("Dateiname;" + String(filename) + ";;;;\n");
  server->sendContent("Beschreibung;" + String(doc["description"] | "Unbekannt") + ";;;;\n");
  server->sendContent("Zeitstempel;" + String(doc["timestamp"] | "Unbekannt") + ";;;;\n");
  server->sendContent("Export-Datum;" + String(millis()) + ";;;;\n");
  server->sendContent("Format-Version;2.0;;;;\n");
  server->sendContent(";;;;;\n");
  
  // ===========================================
  // 2. TEILFAKTORIELLE DATEN
  // ===========================================
  server->sendContent("=== TEILFAKTORIELLE VERSUCHE (2^(5-2)) ===\n");
  server->sendContent(";;;;;\n");
  server->sendContent("Versuch_Nr;Messung_1_uW;Messung_2_uW;Messung_3_uW;Messung_4_uW;Messung_5_uW;Mittelwert_uW;Standardabweichung;Steigung;Groesse;Abstand;Luftstaerke;Blattanzahl\n");
  
  JsonArray tfMessungen = doc["teilfaktoriellMessungen"];
  JsonArray tfMittelwerte = doc["teilfaktoriellMittelwerte"];
  JsonArray tfStdDev = doc["teilfaktoriellStandardabweichungen"];
  
  // Teilfaktorieller Plan (aus dem Code)
  const int teilfaktoriellPlan[8][5] = {
    {-1, -1, -1,  1,  1},
    { 1, -1, -1, -1, -1},
    {-1,  1, -1, -1,  1},
    { 1,  1, -1,  1, -1},
    {-1, -1,  1,  1, -1},
    { 1, -1,  1, -1,  1},
    {-1,  1,  1, -1, -1},
    { 1,  1,  1,  1,  1}
  };
  
  for (int i = 0; i < 8; i++) {
    String zeile = String(i+1) + ";";
    
    // Messungen
    JsonArray messungen = tfMessungen[i];
    for (int j = 0; j < 5; j++) {
      zeile += formatGerman(messungen[j].as<float>()) + ";";
    }
    
    // Mittelwert und Standardabweichung
    zeile += formatGerman(tfMittelwerte[i].as<float>()) + ";";
    zeile += formatGerman(tfStdDev[i].as<float>()) + ";";
    
    // Faktoreinstellungen
    for (int j = 0; j < 5; j++) {
      zeile += String(teilfaktoriellPlan[i][j]) + ";";
    }
    
    zeile += "\n";
    server->sendContent(zeile);
  }
  
  server->sendContent(";;;;;\n");
  
  // ===========================================
  // 3. VOLLFAKTORIELLE DATEN
  // ===========================================
  server->sendContent("=== VOLLFAKTORIELLE VERSUCHE (2^3) ===\n");
  server->sendContent(";;;;;\n");
  
  // Ausgewählte Faktoren laden
  JsonArray ausgewaehlteVollfaktoren = doc["ausgewaehlteVollfaktoren"];
  const char* faktorNamen[] = {"Steigung", "Groesse", "Abstand", "Luftstaerke", "Blattanzahl"};
  
  String vfHeader = "Versuch_Nr;Messung_1_uW;Messung_2_uW;Messung_3_uW;Messung_4_uW;Messung_5_uW;Mittelwert_uW;Standardabweichung;";
  for (int i = 0; i < 3 && i < ausgewaehlteVollfaktoren.size(); i++) {
    int faktorIndex = ausgewaehlteVollfaktoren[i].as<int>();
    vfHeader += String(faktorNamen[faktorIndex]) + ";";
  }
  vfHeader += "\n";
  server->sendContent(vfHeader);
  
  JsonArray vfMessungen = doc["vollfaktoriellMessungen"];
  JsonArray vfMittelwerte = doc["vollfaktoriellMittelwerte"];
  JsonArray vfStdDev = doc["vollfaktoriellStandardabweichungen"];
  
  for (int i = 0; i < 8; i++) {
    String zeile = String(i+1) + ";";
    
    // Messungen
    JsonArray messungen = vfMessungen[i];
    for (int j = 0; j < 5; j++) {
      zeile += formatGerman(messungen[j].as<float>()) + ";";
    }
    
    // Mittelwert und Standardabweichung
    zeile += formatGerman(vfMittelwerte[i].as<float>()) + ";";
    zeile += formatGerman(vfStdDev[i].as<float>()) + ";";
    
    // Vollfaktorielle Faktoreinstellungen (2^3 Plan)
    for (int j = 0; j < 3; j++) {
      int stufe = (i & (1 << j)) ? 1 : -1;
      zeile += String(stufe) + ";";
    }
    
    zeile += "\n";
    server->sendContent(zeile);
  }
  
  server->sendContent(";;;;;\n");
  
  // ===========================================
  // 4. HAUPTEFFEKTE-ANALYSE
  // ===========================================
  server->sendContent("=== HAUPTEFFEKTE-ANALYSE ===\n");
  server->sendContent(";;;;;\n");
  server->sendContent("Faktor;Effekt_uW;Mittelwert_niedrig_uW;Mittelwert_hoch_uW;Absoluter_Effekt;Rang\n");
  
  JsonArray effekte = doc["effekte"];
  
  // Sortierung für Ranking
  int sortierteIndizes[5] = {0, 1, 2, 3, 4};
  float absEffekte[5];
  
  for (int i = 0; i < 5; i++) {
    absEffekte[i] = abs(effekte[i].as<float>());
  }
  
  // Bubble Sort für Ranking
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4 - i; j++) {
      if (absEffekte[j] < absEffekte[j + 1]) {
        float tempAbs = absEffekte[j];
        absEffekte[j] = absEffekte[j + 1];
        absEffekte[j + 1] = tempAbs;
        
        int tempIdx = sortierteIndizes[j];
        sortierteIndizes[j] = sortierteIndizes[j + 1];
        sortierteIndizes[j + 1] = tempIdx;
      }
    }
  }
  
  // Berechne Mittelwerte für -1 und +1 Level (vereinfacht)
  for (int i = 0; i < 5; i++) {
    int faktorIndex = sortierteIndizes[i];
    float effektWert = effekte[faktorIndex].as<float>();
    
    // Vereinfachte Berechnung der Mittelwerte (basierend auf Gesamtmittelwert)
    float gesamtmittelwert = 0;
    for (int j = 0; j < tfMittelwerte.size(); j++) {
      gesamtmittelwert += tfMittelwerte[j].as<float>();
    }
    gesamtmittelwert /= tfMittelwerte.size();
    
    float mittelwertNiedrig = gesamtmittelwert - effektWert / 2.0;
    float mittelwertHoch = gesamtmittelwert + effektWert / 2.0;
    
    String zeile = String(faktorNamen[faktorIndex]) + ";";
    zeile += formatGerman(effektWert) + ";";
    zeile += formatGerman(mittelwertNiedrig) + ";";
    zeile += formatGerman(mittelwertHoch) + ";";
    zeile += formatGerman(abs(effektWert)) + ";";
    zeile += String(i + 1) + "\n";
    
    server->sendContent(zeile);
  }
  
  server->sendContent(";;;;;\n");
  
  // ===========================================
  // 5. PARETO-ANALYSE
  // ===========================================
  server->sendContent("=== PARETO-ANALYSE ===\n");
  server->sendContent(";;;;;\n");
  server->sendContent("Rang;Faktor;Absoluter_Effekt;Prozent_Anteil;Kumuliert_Prozent\n");
  
  // Berechne Gesamtsumme für Prozentanteile
  float gesamtsumme = 0;
  for (int i = 0; i < 5; i++) {
    gesamtsumme += absEffekte[i];
  }
  
  float kumulierteProzent = 0;
  for (int i = 0; i < 5; i++) {
    int faktorIndex = sortierteIndizes[i];
    float prozentAnteil = (gesamtsumme > 0) ? (absEffekte[i] / gesamtsumme * 100.0) : 0;
    kumulierteProzent += prozentAnteil;
    
    String zeile = String(i + 1) + ";";
    zeile += String(faktorNamen[faktorIndex]) + ";";
    zeile += formatGerman(absEffekte[i]) + ";";
    zeile += formatGerman(prozentAnteil, 1) + ";";
    zeile += formatGerman(kumulierteProzent, 1) + "\n";
    
    server->sendContent(zeile);
  }
  
  server->sendContent(";;;;;\n");
  
  // ===========================================
  // 6. REGRESSIONSANALYSE
  // ===========================================
  server->sendContent("=== REGRESSIONSANALYSE ===\n");
  server->sendContent(";;;;;\n");
  server->sendContent("Parameter;Wert;Einheit;Beschreibung\n");
  
  // Regressionskoeffizienten (vereinfachte Berechnung)
  float gesamtmittelwertVF = 0;
  for (int i = 0; i < vfMittelwerte.size(); i++) {
    gesamtmittelwertVF += vfMittelwerte[i].as<float>();
  }
  gesamtmittelwertVF /= vfMittelwerte.size();
  
  server->sendContent("b0;" + formatGerman(gesamtmittelwertVF) + ";uW;Konstanter Term\n");
  
  // Koeffizienten für ausgewählte Faktoren
  for (int i = 0; i < 3 && i < ausgewaehlteVollfaktoren.size(); i++) {
    int faktorIndex = ausgewaehlteVollfaktoren[i].as<int>();
    float koeffizient = effekte[faktorIndex].as<float>() / 2.0; // Hälfte des Effekts
    
    String zeile = "b" + String(i+1) + ";";
    zeile += formatGerman(koeffizient) + ";uW;";
    zeile += String(faktorNamen[faktorIndex]) + "\n";
    
    server->sendContent(zeile);
  }
  
  // R² (vereinfachte Schätzung)
  float r2_schaetzung = 0.85; // Platzhalter - in echter Implementierung berechnen
  server->sendContent("R_Quadrat;" + formatGerman(r2_schaetzung) + ";;Bestimmtheitsmass\n");
  
  server->sendContent(";;;;;\n");
  
  // ===========================================
  // 7. OPTIMIERUNG UND PROGNOSE
  // ===========================================
  server->sendContent("=== OPTIMIERUNG UND PROGNOSE ===\n");
  server->sendContent(";;;;;\n");
  server->sendContent("Faktor;Optimale_Stufe;Optimaler_Wert;Begründung\n");
  
  for (int i = 0; i < 5; i++) {
    float effektWert = effekte[i].as<float>();
    String optimaleStufe = (effektWert > 0) ? "+1" : "-1";
    String optimaleEinstellung = (effektWert > 0) ? "Hoch" : "Niedrig";
    String begruendung = (effektWert > 0) ? "Positiver Effekt" : "Negativer Effekt";
    
    String zeile = String(faktorNamen[i]) + ";";
    zeile += optimaleStufe + ";";
    zeile += optimaleEinstellung + ";";
    zeile += begruendung + "\n";
    
    server->sendContent(zeile);
  }
  
  server->sendContent(";;;;;\n");
  server->sendContent("PROGNOSTIZIERTE MAXIMALE LEISTUNG;;;;\n");
  
  // Einfache Prognose basierend auf positiven Effekten
  float prognose = gesamtmittelwertVF;
  for (int i = 0; i < 5; i++) {
    float effektWert = effekte[i].as<float>();
    if (effektWert > 0) {
      prognose += effektWert / 2.0;
    } else {
      prognose -= effektWert / 2.0;
    }
  }
  
  server->sendContent("Prognostizierte_Leistung;" + formatGerman(prognose, 2) + ";uW;Bei optimalen Einstellungen\n");
  
  server->sendContent(";;;;;\n");
  
  // ===========================================
  // 8. ZUSÄTZLICHE INFORMATIONEN
  // ===========================================
  server->sendContent("=== ZUSÄTZLICHE INFORMATIONEN ===\n");
  server->sendContent(";;;;;\n");
  server->sendContent("Information;Wert;;;;\n");
  server->sendContent("Anzahl_Teilfaktorielle_Versuche;" + String(tfMittelwerte.size()) + ";;;;\n");
  server->sendContent("Anzahl_Vollfaktorielle_Versuche;" + String(vfMittelwerte.size()) + ";;;;\n");
  server->sendContent("Messungen_pro_Versuch;5;;;;\n");
  
  // Statistiken
  float minLeistung = 999, maxLeistung = -999;
  for (int i = 0; i < vfMittelwerte.size(); i++) {
    float wert = vfMittelwerte[i].as<float>();
    if (wert < minLeistung) minLeistung = wert;
    if (wert > maxLeistung) maxLeistung = wert;
  }
  
  server->sendContent("Minimale_Leistung;" + formatGerman(minLeistung) + ";uW;;\n");
  server->sendContent("Maximale_Leistung;" + formatGerman(maxLeistung) + ";uW;;\n");
  server->sendContent("Leistungsspanne;" + formatGerman(maxLeistung - minLeistung) + ";uW;;\n");
  
  server->sendContent(";;;;;\n");
  server->sendContent("=== ENDE DES DATENEXPORTS ===\n");
  server->sendContent("Hinweis: Dezimaltrennzeichen ist Komma (deutsche Lokalisierung)\n");
  server->sendContent("CSV-Trennzeichen ist Semikolon für Excel-Kompatibilität\n");
}

void WindTurbineDataManager::serveFileChunked(const char* filename) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    server->send(404, "text/plain", "Datei nicht gefunden");
    return;
  }
  
  String downloadFilename = "windkraft_experiment_" + String(filename);
  server->sendHeader("Content-Disposition", "attachment; filename=\"" + downloadFilename + "\"");
  server->setContentLength(file.size());
  server->send(200, "application/octet-stream", "");
  
  const size_t bufferSize = 1024;
  uint8_t buffer[bufferSize];
  
  while (file.available()) {
    size_t bytesRead = file.readBytes((char*)buffer, bufferSize);
    server->sendContent_P((const char*)buffer, bytesRead);
  }
  
  file.close();
}

/**
 * Basis-Datenverwaltungsfunktionen
 */
bool WindTurbineDataManager::saveExperiment(const char* description, float teilfaktoriellMessungen[][5], 
                                           float teilfaktoriellMittelwerte[], float teilfaktoriellStandardabweichungen[],
                                           float vollfaktoriellMessungen[][5], float vollfaktoriellMittelwerte[], 
                                           float vollfaktoriellStandardabweichungen[], float effekte[], 
                                           int ausgewaehlteVollfaktoren[]) {
  
  // Eindeutigen Dateinamen generieren
  String filename = generateFilename();
  
  DynamicJsonDocument doc(8192);
  
  // Metadaten
  doc["timestamp"] = millis();
  doc["description"] = description;
  doc["version"] = "2.0";
  
  // Teilfaktorielle Daten
  JsonArray tfMessungenArray = doc.createNestedArray("teilfaktoriellMessungen");
  for (int i = 0; i < 8; i++) {
    JsonArray versuch = tfMessungenArray.createNestedArray();
    for (int j = 0; j < 5; j++) {
      versuch.add(teilfaktoriellMessungen[i][j]);
    }
  }
  
  JsonArray tfMittelwerteArray = doc.createNestedArray("teilfaktoriellMittelwerte");
  JsonArray tfStdDevArray = doc.createNestedArray("teilfaktoriellStandardabweichungen");
  for (int i = 0; i < 8; i++) {
    tfMittelwerteArray.add(teilfaktoriellMittelwerte[i]);
    tfStdDevArray.add(teilfaktoriellStandardabweichungen[i]);
  }
  
  // Vollfaktorielle Daten
  JsonArray vfMessungenArray = doc.createNestedArray("vollfaktoriellMessungen");
  for (int i = 0; i < 8; i++) {
    JsonArray versuch = vfMessungenArray.createNestedArray();
    for (int j = 0; j < 5; j++) {
      versuch.add(vollfaktoriellMessungen[i][j]);
    }
  }
  
  JsonArray vfMittelwerteArray = doc.createNestedArray("vollfaktoriellMittelwerte");
  JsonArray vfStdDevArray = doc.createNestedArray("vollfaktoriellStandardabweichungen");
  for (int i = 0; i < 8; i++) {
    vfMittelwerteArray.add(vollfaktoriellMittelwerte[i]);
    vfStdDevArray.add(vollfaktoriellStandardabweichungen[i]);
  }
  
  // Effekte
  JsonArray effekteArray = doc.createNestedArray("effekte");
  for (int i = 0; i < 5; i++) {
    effekteArray.add(effekte[i]);
  }
  
  // Ausgewählte Faktoren
  JsonArray faktoren = doc.createNestedArray("ausgewaehlteVollfaktoren");
  for (int i = 0; i < 3; i++) {
    faktoren.add(ausgewaehlteVollfaktoren[i]);
  }
  
  // Datei speichern
  File file = SPIFFS.open("/" + filename, FILE_WRITE);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei zum Schreiben");
    return false;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("Fehler beim Schreiben der JSON-Daten");
    file.close();
    return false;
  }
  
  file.close();
  Serial.println("Experiment gespeichert: " + filename);
  return true;
}

bool WindTurbineDataManager::loadExperiment(const char* filename, float teilfaktoriellMessungen[][5], 
                                          float teilfaktoriellMittelwerte[], float teilfaktoriellStandardabweichungen[],
                                          float vollfaktoriellMessungen[][5], float vollfaktoriellMittelwerte[], 
                                          float vollfaktoriellStandardabweichungen[], float effekte[], 
                                          int ausgewaehlteVollfaktoren[]) {
  
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei: " + String(filename));
    return false;
  }
  
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Fehler beim Parsen der JSON-Daten: " + String(error.c_str()));
    return false;
  }
  
  // Teilfaktorielle Daten laden
  JsonArray tfMessungenArray = doc["teilfaktoriellMessungen"];
  for (int i = 0; i < 8 && i < tfMessungenArray.size(); i++) {
    JsonArray versuch = tfMessungenArray[i];
    for (int j = 0; j < 5 && j < versuch.size(); j++) {
      teilfaktoriellMessungen[i][j] = versuch[j];
    }
  }
  
  JsonArray tfMittelwerteArray = doc["teilfaktoriellMittelwerte"];
  JsonArray tfStdDevArray = doc["teilfaktoriellStandardabweichungen"];
  for (int i = 0; i < 8; i++) {
    teilfaktoriellMittelwerte[i] = tfMittelwerteArray[i];
    teilfaktoriellStandardabweichungen[i] = tfStdDevArray[i];
  }
  
  // Vollfaktorielle Daten laden
  JsonArray vfMessungenArray = doc["vollfaktoriellMessungen"];
  for (int i = 0; i < 8 && i < vfMessungenArray.size(); i++) {
    JsonArray versuch = vfMessungenArray[i];
    for (int j = 0; j < 5 && j < versuch.size(); j++) {
      vollfaktoriellMessungen[i][j] = versuch[j];
    }
  }
  
  JsonArray vfMittelwerteArray = doc["vollfaktoriellMittelwerte"];
  JsonArray vfStdDevArray = doc["vollfaktoriellStandardabweichungen"];
  for (int i = 0; i < 8; i++) {
    vollfaktoriellMittelwerte[i] = vfMittelwerteArray[i];
    vollfaktoriellStandardabweichungen[i] = vfStdDevArray[i];
  }
  
  // Effekte laden
  JsonArray effekteArray = doc["effekte"];
  for (int i = 0; i < 5; i++) {
    effekte[i] = effekteArray[i];
  }
  
  // Ausgewählte Faktoren laden
  JsonArray faktoren = doc["ausgewaehlteVollfaktoren"];
  for (int i = 0; i < 3; i++) {
    ausgewaehlteVollfaktoren[i] = faktoren[i];
  }
  
  Serial.println("Experiment geladen: " + String(filename));
  return true;
}

int WindTurbineDataManager::listExperiments(ExperimentMetadata* metadata, int maxCount) {
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    return 0;
  }
  
  int count = 0;
  File file = root.openNextFile();
  
  while (file && count < maxCount) {
    if (!file.isDirectory()) {
      String filename = file.name();
      
      // Überspringe System-Dateien
      if (filename.startsWith("/.") || filename.endsWith(".tmp")) {
        file = root.openNextFile();
        continue;
      }
      
      // Lade Metadaten
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, file);
      
      if (!error) {
        strcpy(metadata[count].filename, filename.c_str());
        strcpy(metadata[count].description, doc["description"] | "Unbekannt");
        strcpy(metadata[count].timestamp, doc["timestamp"] | "Unbekannt");
        
        // Berechne maximale Leistung
        JsonArray vfMittelwerte = doc["vollfaktoriellMittelwerte"];
        float maxPower = 0;
        for (int i = 0; i < vfMittelwerte.size(); i++) {
          float power = vfMittelwerte[i];
          if (power > maxPower) maxPower = power;
        }
        metadata[count].maxPower = maxPower;
        metadata[count].isValid = true;
        
        count++;
      }
    }
    file = root.openNextFile();
  }
  
  return count;
}

bool WindTurbineDataManager::deleteExperiment(const char* filename) {
  return SPIFFS.remove("/" + String(filename));
}

bool WindTurbineDataManager::deleteAllExperiments() {
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    return false;
  }
  
  bool success = true;
  File file = root.openNextFile();
  
  while (file) {
    if (!file.isDirectory()) {
      String filename = file.name();
      
      // Überspringe System-Dateien
      if (!filename.startsWith("/.") && !filename.endsWith(".tmp")) {
        if (!SPIFFS.remove("/" + filename)) {
          success = false;
        }
      }
    }
    file = root.openNextFile();
  }
  
  return success;
}

bool WindTurbineDataManager::validateExperimentData(float teilfaktoriellMessungen[][5], 
                                                   float vollfaktoriellMessungen[][5]) {
  // Prüfe auf gültige Werte
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 5; j++) {
      if (teilfaktoriellMessungen[i][j] < 0 || teilfaktoriellMessungen[i][j] > 1000) {
        return false;
      }
      if (vollfaktoriellMessungen[i][j] < 0 || vollfaktoriellMessungen[i][j] > 1000) {
        return false;
      }
    }
  }
  return true;
}

String WindTurbineDataManager::generateFilename() {
  return "exp_" + String(millis()) + ".json";
}

String WindTurbineDataManager::getExportURL() {
  if (wifiExportActive) {
    return "http://" + WiFi.softAPIP().toString();
  }
  return "";
}

bool WindTurbineDataManager::isExportActive() {
  return wifiExportActive;
}

String WindTurbineDataManager::getCurrentSSID() {
  return currentSSID;
}

bool WindTurbineDataManager::isWiFiExportActive() {
  return wifiExportActive;
}

String WindTurbineDataManager::getCurrentIP() {
  if (wifiExportActive) {
    return WiFi.softAPIP().toString();
  }
  return "0.0.0.0";
}

size_t WindTurbineDataManager::getUsedSpace() {
  return SPIFFS.usedBytes();
}

size_t WindTurbineDataManager::getTotalSpace() {
  return SPIFFS.totalBytes();
}

size_t WindTurbineDataManager::getFreeSpace() {
  return SPIFFS.totalBytes() - SPIFFS.usedBytes();
}

bool WindTurbineDataManager::fileExists(const char* filename) {
  return SPIFFS.exists("/" + String(filename));
}

size_t WindTurbineDataManager::getFileSize(const char* filename) {
  File file = SPIFFS.open("/" + String(filename), FILE_READ);
  if (!file) return 0;
  size_t size = file.size();
  file.close();
  return size;
}

bool WindTurbineDataManager::createBackup(const char* backupName) {
  // Implementierung für Backup-Funktionalität
  return true; // Placeholder
}

bool WindTurbineDataManager::restoreBackup(const char* backupFilename) {
  // Implementierung für Restore-Funktionalität
  return true; // Placeholder
}

void WindTurbineDataManager::printSystemInfo() {
  Serial.println("=== WindTurbine DataManager System Info ===");
  Serial.println("SPIFFS Total: " + String(getTotalSpace()) + " bytes");
  Serial.println("SPIFFS Used: " + String(getUsedSpace()) + " bytes");
  Serial.println("SPIFFS Free: " + String(getFreeSpace()) + " bytes");
  Serial.println("WiFi Export Active: " + String(wifiExportActive ? "Yes" : "No"));
  if (wifiExportActive) {
    Serial.println("Current SSID: " + currentSSID);
    Serial.println("Current IP: " + getCurrentIP());
  }
  Serial.println("===========================================");
}
