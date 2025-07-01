// User_Setup.h - Konfigurationsdatei für TFT_eSPI mit ST7796S 4-Inch Display
// Kopieren Sie den Inhalt dieser Datei in die User_Setup.h im TFT_eSPI Bibliotheksordner
// Die Datei sollte sich befinden unter:
// C:\Users\[Benutzername]\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h
// oder
// C:\Users\[Benutzername]\AppData\Local\Arduino15\libraries\TFT_eSPI\User_Setup.h

// ##################################################################################
//
// Abschnitt 1. Definition des Display-Treibers
// WICHTIG: Stellen Sie sicher, dass alle anderen Display-Treiber auskommentiert sind
//
// ##################################################################################

#define ST7796_DRIVER     // Aktiviert den ST7796S Treiber

// ##################################################################################
//
// Abschnitt 2. Definition der ESP32 Pins für das Display
//
// ##################################################################################

// Pins für Control Signale - KORRIGIERT ENTSPRECHEND WindTurbineConstants.h:
#define TFT_CS   15    // Chip Select pin
#define TFT_DC   2     // Data/Command pin
#define TFT_RST  4     // Reset pin

// SPI-Pins für ESP32 VSPI bus
#define TFT_MOSI 23    // SPI MOSI pin
#define TFT_MISO 19    // SPI MISO pin (kann auf -1 gesetzt werden, wenn nicht verwendet)
#define TFT_SCLK 18    // SPI Clock pin

// Hintergrundbeleuchtung
#define TFT_BL   32    // Hintergrundbeleuchtung (TFT_LED in deinem Code)
#define TFT_BACKLIGHT_ON HIGH  // HIGH oder LOW je nach Display

// ##################################################################################
//
// Abschnitt 3. Display-Auflösung definieren
//
// ##################################################################################

// 4-Inch ST7796S Display hat typischerweise eine Auflösung von 320x480
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// ##################################################################################
//
// Abschnitt 4. SPI-Frequenzen definieren
//
// ##################################################################################

// Standard-SPI-Frequenz für Zugriffe auf das Display
#define SPI_FREQUENCY  27000000   // 27 MHz für ST7796S

// Frequenz für Lesezugriffe (normalerweise niedriger)
#define SPI_READ_FREQUENCY  20000000

// Frequenz für Touch-Controller falls vorhanden
#define SPI_TOUCH_FREQUENCY  2500000

// ##################################################################################
//
// Abschnitt 5. Schriftarten laden (Kommentieren Sie die aus, die Sie nicht benötigen)
//
// ##################################################################################

#define LOAD_GLCD   // Standard-Adafruit-Schriftart 8x8
#define LOAD_FONT2  // Small 16 pixel high font
#define LOAD_FONT4  // Medium 26 pixel high font
#define LOAD_FONT6  // Large 48 pixel high font
#define LOAD_FONT7  // 7-segment 48 pixel high font
#define LOAD_FONT8  // Large 75 pixel high font
//#define LOAD_GFXFF  // FreeFonts (benötigen mehr Speicher)

// ##################################################################################
//
// Abschnitt 6. Farbeinstellungen und weitere Optionen
//
// ##################################################################################

// Definiert die SPI-Betriebsart - ST7796 funktioniert normalerweise mit SPI_MODE3
#define TFT_SPI_MODE SPI_MODE3

// Wenn die Farben falsch erscheinen, versuchen Sie die RGB-Reihenfolge zu ändern
//#define TFT_RGB_ORDER TFT_RGB  // Standard: Farbe: rrrrrggg gggbbbbb
//#define TFT_RGB_ORDER TFT_BGR  // Alternative: Farbe: bbbbbggg gggrrrrr

// Support für Transaktionen beibehalten
#define SUPPORT_TRANSACTIONS

// ##################################################################################
//
// Hinweise zur Fehlerbehebung:
//
// 1. Falls das Display schwarz bleibt, überprüfen Sie:
//    - Kabelverbindungen
//    - Stromversorgung des Displays (3.3V)
//    - Pins in der Definition oben
//
// 2. Falls falsche Farben erscheinen:
//    - Kommentieren Sie die TFT_RGB_ORDER TFT_BGR Zeile ein
//
// 3. Falls das Bild falsch ausgerichtet ist:
//    - Verwenden Sie im Sketch: tft.setRotation(0, 1, 2 oder 3);
//
// ##################################################################################