/**
 * WindTurbineConstants.h
 * Konstanten und Definitionen für das Windkraftanlagen-Experiment
 */

 #ifndef WIND_TURBINE_CONSTANTS_H
 #define WIND_TURBINE_CONSTANTS_H
 
 // Datenverwaltungskonstanten
 #define MAX_SAVED_EXPERIMENTS 20  // Maximale Anzahl gespeicherter Experimente
 
 // Pin-Definitionen für den Encoder
 #define ENCODER_PIN_A 34  // CLK-Pin des KY-040 Encoders
 #define ENCODER_PIN_B 35  // DT-Pin des KY-040 Encoders
 #define ENCODER_BUTTON 16 // SW-Pin (Button) des KY-040 Encoders
 
 // Pin-Definitionen für den INA226 Leistungssensor
 // Der INA226 verwendet I2C, daher benötigen wir nur SDA und SCL
 #define INA226_SDA 21     // I2C Datenleitung
 #define INA226_SCL 22     // I2C Taktleitung
 
 // Pin-Definitionen für das TFT-Display
 #define TFT_CS   15       // Chip Select
 #define TFT_RESET 4       // Reset
 #define TFT_DC    2       // Data/Command
 #define TFT_MOSI 23       // SPI MOSI
 #define TFT_SCK  18       // SPI Clock
 #define TFT_LED  32       // Hintergrundbeleuchtung
 
 // Keypad-Konfiguration
 #define KEYPAD_ROWS 4
 #define KEYPAD_COLS 4
 
 // Moderne Farbpalette definieren
 #define TFT_BACKGROUND 0x0841      // Dunkelblau (statt schwarz)
 #define TFT_BLACK 0x0000           // Schwarz
 #define TFT_WHITE 0xFFFF           // Weiß
 #define TFT_TEXT 0xFFFF           // Weiß (unverändert)
 #define TFT_HEADER 0x04FF         // Helles Blau (moderner Blauton)
 #define TFT_HIGHLIGHT 0x07FF      // Türkis (etwas intensiver)
 #define TFT_WARNING 0xF844        // Helleres Rot (weniger grell)
 #define TFT_SUCCESS 0x0740        // Dunkleres Grün (naturnaher)
 #define TFT_GRID 0x39C7           // Helleres Grau (besser sichtbar)
 #define TFT_LIGHT_TEXT 0xBDF7     // Hellgrau (besserer Kontrast)
 
 // Neue visuelle Elemente definieren
 #define TFT_CHART_BAR 0x05B6      // Hellblau für Balkendiagramme
 #define TFT_CHART_ACCENT 0xFD20   // Akzentfarbe (Orange) für Hervorhebungen
 #define TFT_OUTLINE 0x7BEF        // Rahmenfarbe
 #define TFT_TITLE_BG 0x0861       // Titelbalken-Hintergrund
 #define TFT_SUBTITLE 0xAD75       // Untertitel-Farbe
 #define TFT_STATUS_BAR 0x0882     // Statusleisten-Hintergrund
 
 // Layoutkonstanten
 #define STATUS_BAR_HEIGHT 20      // Höhe der Statusleiste
 #define HEADER_HEIGHT 30          // Höhe des Titelbalkens
 #define CONTENT_MARGIN 5          // Standardrand für Inhalte
 #define CHART_MARGIN 8            // Rand für Diagramme
 
 // Faktornamen und Stufen
 extern const char* faktorNamen[];
 extern const char* faktorEinheitenNiedrig[];
 extern const char* faktorEinheitenHoch[];
 
 // Versuchspläne
 // Teil- und vollfaktorieller Versuchsplan werden als 2D-Arrays kodiert
 // -1 = niedrige Stufe, 1 = hohe Stufe, 0 = nicht verwendet
 // Teilfaktorieller Plan (2^(5-2) = 8 Versuche)
 extern const int teilfaktoriellPlan[8][5];
 
 #endif // WIND_TURBINE_CONSTANTS_H
