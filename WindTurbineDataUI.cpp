/**
 * WindTurbineDataUI.cpp
 * UI-Funktionen für die Datenverwaltung des Windkraftanlagen-Experiments
 */

#include "WindTurbineExperiment.h"

/**
 * Zeigt einen Bildschirm zur Eingabe einer Versuchsbeschreibung an
 * Ermöglicht die Texteingabe über das Keypad mit verschiedenen Zeichenmodi
 */
void WindTurbineExperiment::zeigeBeschreibungEingabe() {
  tft.fillScreen(TFT_BACKGROUND);
  
  // Titelbereich
  zeichneTitelbalken("Versuchsbeschreibung eingeben");
  
  // Eingabebereich
  tft.fillRoundRect(20, 50, 440, 200, 5, TFT_OUTLINE);
  
  // Aktuelle Eingabe anzeigen
  tft.setTextColor(TFT_TEXT);
  tft.setTextSize(1);
  tft.setCursor(30, 60);
  tft.println("Geben Sie eine Beschreibung für diesen Versuch ein:");
  
  // Eingabefeld
  tft.fillRoundRect(30, 80, 420, 160, 5, TFT_BACKGROUND);
  tft.drawRoundRect(30, 80, 420, 160, 5, TFT_HIGHLIGHT);
  
  // Aktuelle Eingabe anzeigen
  tft.setCursor(40, 90);
  tft.setTextColor(TFT_TEXT);
  tft.print(textEingabe);
  
  // Cursor anzeigen
  int cursorX = 40 + (strlen(textEingabe) % 40) * 6;
  int cursorY = 90 + (strlen(textEingabe) / 40) * 16;
  tft.fillRect(cursorX, cursorY, 6, 12, TFT_HIGHLIGHT);
  
  // Tastatur-Anleitung
  tft.fillRoundRect(20, 260, 440, 40, 5, TFT_SUBTITLE);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 270);
  tft.println("Keypad: 1-9=Buchstaben, *=Leerzeichen, #=Speichern, D=Abbrechen");
  tft.setCursor(30, 285);
  tft.println("A=Löschen, B=Groß/Klein, C=Sonderzeichen");
  
  // Anleitung
  zeichneStatusleiste("Geben Sie eine Beschreibung ein und drücken Sie # zum Speichern");
  
  // Variablen für die Texteingabe
  bool grossbuchstaben = false;
  bool sonderzeichen = false;
  bool fertig = false;
  
  // Eingabeschleife
  while (!fertig) {
    char key = keypad.getKey();
    
    if (key) {
      if (key == '#') {
        // Speichern und beenden
        fertig = true;
        
        // Versuch speichern
        if (dataManager.saveExperiment(textEingabe, teilfaktoriellMessungen, 
                                      teilfaktoriellMittelwerte, teilfaktoriellStandardabweichungen,
                                      vollfaktoriellMessungen, vollfaktoriellMittelwerte, 
                                      vollfaktoriellStandardabweichungen, effekte, 
                                      ausgewaehlteVollfaktoren)) {
          // Erfolgsmeldung anzeigen
          tft.fillScreen(TFT_BACKGROUND);
          tft.fillRoundRect(90, 120, 300, 80, 8, TFT_SUCCESS);
          tft.setTextColor(TFT_TEXT);
          tft.setTextSize(1);
          tft.setCursor(110, 140);
          tft.println("Versuch erfolgreich gespeichert!");
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
          
          // Zurück zur Zusammenfassung
          zeigeZusammenfassung();
        } else {
          // Fehlermeldung anzeigen
          tft.fillScreen(TFT_BACKGROUND);
          tft.fillRoundRect(90, 120, 300, 80, 8, TFT_WARNING);
          tft.setTextColor(TFT_TEXT);
          tft.setTextSize(1);
          tft.setCursor(110, 140);
          tft.println("Fehler beim Speichern des Versuchs!");
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
          
          // Zurück zur Zusammenfassung
          zeigeZusammenfassung();
        }
      } else if (key == 'D') {
        // Abbrechen
        fertig = true;
        
        // Zurück zur Zusammenfassung
        zeigeZusammenfassung();
      } else if (key == 'A' && strlen(textEingabe) > 0) {
        // Letztes Zeichen löschen
        textEingabe[strlen(textEingabe) - 1] = '\0';
        
        // UI aktualisieren
        zeigeBeschreibungEingabe();
      } else if (key == 'B') {
        // Groß-/Kleinschreibung umschalten
        grossbuchstaben = !grossbuchstaben;
        sonderzeichen = false;
        
        // Statusleiste aktualisieren
        tft.fillRect(0, 320-STATUS_BAR_HEIGHT, 400, STATUS_BAR_HEIGHT, TFT_STATUS_BAR);
        tft.setTextColor(TFT_TEXT);
        tft.setCursor(10, 320-15);
        tft.print(grossbuchstaben ? "GROSSBUCHSTABEN" : "kleinbuchstaben");
      } else if (key == 'C') {
        // Sonderzeichen umschalten
        sonderzeichen = !sonderzeichen;
        grossbuchstaben = false;
        
        // Statusleiste aktualisieren
        tft.fillRect(0, 320-STATUS_BAR_HEIGHT, 400, STATUS_BAR_HEIGHT, TFT_STATUS_BAR);
        tft.setTextColor(TFT_TEXT);
        tft.setCursor(10, 320-15);
        tft.print(sonderzeichen ? "SONDERZEICHEN" : "normale Zeichen");
      } else if (key == '*') {
        // Leerzeichen
        if (strlen(textEingabe) < 99) {
          textEingabe[strlen(textEingabe)] = ' ';
          textEingabe[strlen(textEingabe) + 1] = '\0';
          
          // UI aktualisieren
          zeigeBeschreibungEingabe();
        }
      } else if (key >= '1' && key <= '9') {
        // Zeichen hinzufügen
        if (strlen(textEingabe) < 99) {
          char newChar;
          
          if (sonderzeichen) {
            // Sonderzeichen
            switch (key) {
              case '1': newChar = '.'; break;
              case '2': newChar = ','; break;
              case '3': newChar = '!'; break;
              case '4': newChar = '?'; break;
              case '5': newChar = '-'; break;
              case '6': newChar = '+'; break;
              case '7': newChar = '='; break;
              case '8': newChar = '/'; break;
              case '9': newChar = '&'; break;
              default: newChar = ' ';
            }
          } else {
            // Buchstaben (Multi-tap wie bei alten Handys)
            static char lastKey = 0;
            static unsigned long lastKeyTime = 0;
            static int keyPressCount = 0;
            
            // Prüfen, ob es sich um eine neue Taste handelt oder die Zeit abgelaufen ist
            if (key != lastKey || (millis() - lastKeyTime > 1000)) {
              keyPressCount = 0;
            }
            
            // Tastendruck zählen
            keyPressCount = (keyPressCount + 1) % 4;
            
            // Zeichen basierend auf Taste und Anzahl der Tastendrücke bestimmen
            char chars[9][4] = {
              {'a', 'b', 'c', 'a'}, // 1
              {'d', 'e', 'f', 'd'}, // 2
              {'g', 'h', 'i', 'g'}, // 3
              {'j', 'k', 'l', 'j'}, // 4
              {'m', 'n', 'o', 'm'}, // 5
              {'p', 'q', 'r', 'p'}, // 6
              {'s', 't', 'u', 's'}, // 7
              {'v', 'w', 'x', 'v'}, // 8
              {'y', 'z', '0', 'y'}  // 9
            };
            
            newChar = chars[key - '1'][keyPressCount];
            
            // Großbuchstaben, wenn aktiviert
            if (grossbuchstaben) {
              newChar = toupper(newChar);
            }
            
            // Aktuelle Taste und Zeit speichern
            lastKey = key;
            lastKeyTime = millis();
            
            // Wenn es sich um denselben Buchstaben handelt, den letzten löschen
            if (keyPressCount > 0 && strlen(textEingabe) > 0) {
              textEingabe[strlen(textEingabe) - 1] = '\0';
            }
          }
          
          // Zeichen hinzufügen
          textEingabe[strlen(textEingabe)] = newChar;
          textEingabe[strlen(textEingabe) + 1] = '\0';
          
          // UI aktualisieren
          zeigeBeschreibungEingabe();
        }
      }
    }
    
    // Auf Encoder-Button prüfen
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
        buttonPressed = true;
        lastDebounceTime = millis();
        
        // Speichern und beenden
        fertig = true;
        
        // Versuch speichern
        if (dataManager.saveExperiment(textEingabe, teilfaktoriellMessungen, 
                                      teilfaktoriellMittelwerte, teilfaktoriellStandardabweichungen,
                                      vollfaktoriellMessungen, vollfaktoriellMittelwerte, 
                                      vollfaktoriellStandardabweichungen, effekte, 
                                      ausgewaehlteVollfaktoren)) {
          // Erfolgsmeldung anzeigen
          tft.fillScreen(TFT_BACKGROUND);
          tft.fillRoundRect(90, 120, 300, 80, 8, TFT_SUCCESS);
          tft.setTextColor(TFT_TEXT);
          tft.setTextSize(1);
          tft.setCursor(110, 140);
          tft.println("Versuch erfolgreich gespeichert!");
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
          
          // Zurück zur Zusammenfassung
          zeigeZusammenfassung();
        }
      }
    } else {
      buttonPressed = false;
    }
  }
}

/**
 * Zeigt eine Liste aller gespeicherten Versuchsdaten an
 * Ermöglicht die Auswahl eines Versuchs zur detaillierten Ansicht
 */
void WindTurbineExperiment::zeigeGespeicherteVersuche() {
  tft.fillScreen(TFT_BACKGROUND);
  
  // Titelbereich
  zeichneTitelbalken("Gespeicherte Versuche");
  
  // Versuche auflisten
  anzahlGespeicherteVersuche = dataManager.listExperiments(gespeicherteVersuche, MAX_SAVED_EXPERIMENTS);
  
  if (anzahlGespeicherteVersuche == 0) {
    // Keine gespeicherten Versuche
    tft.fillRoundRect(90, 120, 300, 80, 8, TFT_OUTLINE);
    tft.setTextColor(TFT_TEXT);
    tft.setTextSize(1);
    tft.setCursor(110, 140);
    tft.println("Keine gespeicherten Versuche vorhanden.");
    tft.setCursor(110, 160);
    tft.println("Drücken Sie eine Taste, um zurückzukehren...");
    
    // Warten auf Tastendruck
    bool warten = true;
    while (warten) {
      char key = keypad.getKey();
      if (key) {
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
    return;
  }
  
  // Versuche anzeigen
  tft.fillRoundRect(20, 50, 440, 220, 5, TFT_OUTLINE);
  
  // Überschriften
  tft.fillRect(21, 51, 438, 20, TFT_TITLE_BG);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setTextSize(1);
  tft.setCursor(30, 56);
  tft.print("Nr.");
  tft.setCursor(60, 56);
  tft.print("Beschreibung");
  tft.setCursor(280, 56);
  tft.print("Datum");
  tft.setCursor(400, 56);
  tft.print("Max. P");
  
  // Versuche auflisten
  for (int i = 0; i < anzahlGespeicherteVersuche && i < 8; i++) {
    int y = 80 + i * 20;
    
    // Zeilenhintergrund
    if (i % 2 == 0) {
      tft.fillRect(21, y-9, 438, 19, 0x1082);
    }
    
    // Nummer
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(30, y);
    tft.print(i + 1);
    
    // Beschreibung (gekürzt)
    tft.setCursor(60, y);
    char kurzeBeschreibung[25];
    strncpy(kurzeBeschreibung, gespeicherteVersuche[i].description, 24);
    kurzeBeschreibung[24] = '\0';
    tft.print(kurzeBeschreibung);
    
    // Datum (gekürzt)
    tft.setCursor(280, y);
    char kurzesDatum[15];
    strncpy(kurzesDatum, gespeicherteVersuche[i].timestamp, 14);
    kurzesDatum[14] = '\0';
    tft.print(kurzesDatum);
    
    // Maximale Leistung
    tft.setCursor(400, y);
    tft.print(gespeicherteVersuche[i].maxPower, 1);
  }
  
  // Anleitung
  tft.fillRoundRect(20, 280, 440, 20, 5, TFT_SUBTITLE);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 285);
  tft.print("Waehlen Sie einen Versuch (1-");
  tft.print(anzahlGespeicherteVersuche);
  tft.print(") oder druecken Sie D zum Zurueckkehren");
  
  zeichneStatusleiste("1-9=Versuch auswaehlen, D=Zurueck");
  
  // Auf Auswahl warten
  bool fertig = false;
  while (!fertig) {
    char key = keypad.getKey();
    
    if (key) {
     if (key == 'D') {
        // Zurück zum vorherigen Bildschirm
        fertig = true;
        zurueckZumVorherigenModus();
      } else if (key >= '1' && key <= '9') {
        int index = key - '1';
        if (index < anzahlGespeicherteVersuche) {
          // Versuch auswählen
          fertig = true;
          strcpy(aktuellerVersuchsFilename, gespeicherteVersuche[index].filename);
          zeigeVersuchDetails(aktuellerVersuchsFilename);
        }
      }
    }
    
    // Auf Encoder-Button prüfen
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
        buttonPressed = true;
        lastDebounceTime = millis();
        
        // Zurück zum vorherigen Bildschirm
        fertig = true;
        zurueckZumVorherigenModus();
      }
    } else {
      buttonPressed = false;
    }
  }
}

/**
 * Zeigt detaillierte Informationen zu einem ausgewählten Versuch an
 * Bietet Optionen zum Exportieren oder Löschen der Versuchsdaten
 * @param filename Dateiname des ausgewählten Versuchs
 */
void WindTurbineExperiment::zeigeVersuchDetails(const char* filename) {
  tft.fillScreen(TFT_BACKGROUND);
  
  // Titelbereich
  zeichneTitelbalken("Versuchsdetails");
  
  // Versuch laden
  float tempTeilfaktoriellMessungen[8][5];
  float tempTeilfaktoriellMittelwerte[8];
  float tempTeilfaktoriellStandardabweichungen[8];
  float tempVollfaktoriellMessungen[8][5];
  float tempVollfaktoriellMittelwerte[8];
  float tempVollfaktoriellStandardabweichungen[8];
  float tempEffekte[5];
  int tempAusgewaehlteVollfaktoren[3];
  
  if (!dataManager.loadExperiment(filename, tempTeilfaktoriellMessungen, 
                                 tempTeilfaktoriellMittelwerte, tempTeilfaktoriellStandardabweichungen,
                                 tempVollfaktoriellMessungen, tempVollfaktoriellMittelwerte, 
                                 tempVollfaktoriellStandardabweichungen, tempEffekte, 
                                 tempAusgewaehlteVollfaktoren)) {
    // Fehler beim Laden
    tft.fillRoundRect(90, 120, 300, 80, 8, TFT_WARNING);
    tft.setTextColor(TFT_TEXT);
    tft.setTextSize(1);
    tft.setCursor(110, 140);
    tft.println("Fehler beim Laden des Versuchs!");
    tft.setCursor(110, 160);
    tft.println("Drücken Sie eine Taste, um zurückzukehren...");
    
    // Warten auf Tastendruck
    bool warten = true;
    while (warten) {
      char key = keypad.getKey();
      if (key) {
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
    
    // Zurück zur Liste der gespeicherten Versuche
    zeigeGespeicherteVersuche();
    return;
  }
  
  // Versuchsdetails anzeigen
  tft.fillRoundRect(20, 50, 440, 220, 5, TFT_OUTLINE);
  
  // Überschrift
  tft.fillRect(21, 51, 438, 20, TFT_TITLE_BG);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setTextSize(1);
  tft.setCursor(30, 56);
  tft.print("Versuchsdetails: ");
  tft.print(filename);
  
  // Beschreibung
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 80);
  tft.print("Beschreibung: ");
  
  // Beschreibung aus Metadaten suchen
  for (int i = 0; i < anzahlGespeicherteVersuche; i++) {
    if (strcmp(gespeicherteVersuche[i].filename, filename) == 0) {
      tft.print(gespeicherteVersuche[i].description);
      break;
    }
  }
  
  // Beste Leistung
  float bestePower = 0;
  for (int i = 0; i < 8; i++) {
    if (tempVollfaktoriellMittelwerte[i] > bestePower) {
      bestePower = tempVollfaktoriellMittelwerte[i];
    }
  }
  
  tft.setCursor(30, 100);
  tft.print("Beste Leistung: ");
  tft.print(bestePower, 2);
  tft.print(" uW");
  
  // Wichtigste Effekte
  tft.setCursor(30, 120);
  tft.print("Wichtigste Faktoren:");
  
  // Effekte sortieren
  int effektIndizes[5] = {0, 1, 2, 3, 4};
  float effektWerte[5];
  for (int i = 0; i < 5; i++) {
    effektWerte[i] = abs(tempEffekte[i]);
  }
  
  // Einfache Sortierung (Bubble Sort)
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4 - i; j++) {
      if (effektWerte[j] < effektWerte[j + 1]) {
        // Werte tauschen
        float tempWert = effektWerte[j];
        effektWerte[j] = effektWerte[j + 1];
        effektWerte[j + 1] = tempWert;
        
        // Indizes tauschen
        int tempIndex = effektIndizes[j];
        effektIndizes[j] = effektIndizes[j + 1];
        effektIndizes[j + 1] = tempIndex;
      }
    }
  }
  
  // Top 3 Faktoren anzeigen
  const char* faktorNamen[] = {"Steigung", "Groesse", "Abstand", "Luftstaerke", "Blattanzahl"};
  for (int i = 0; i < 3; i++) {
    tft.setCursor(50, 140 + i * 20);
    tft.print(i + 1);
    tft.print(". ");
    tft.print(faktorNamen[effektIndizes[i]]);
    tft.print(": ");
    tft.print(tempEffekte[effektIndizes[i]], 2);
  }
  
  // Optionen
  tft.fillRoundRect(20, 280, 440, 20, 5, TFT_SUBTITLE);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 285);
  tft.print("1=Exportieren, 2=Löschen, D=Zurück");
  
  zeichneStatusleiste("1=WiFi-Export, 2=Löschen, D=Zurück");
  
  // Auf Auswahl warten
  bool fertig = false;
  while (!fertig) {
    char key = keypad.getKey();
    
    if (key) {
      if (key == 'D') {
        // Zurück zur Liste der gespeicherten Versuche
        fertig = true;
        zeigeGespeicherteVersuche();
      } else if (key == '1') {
        // WiFi-Export starten
        fertig = true;
        zeigeWiFiExport();
      } else if (key == '2') {
        // Versuch löschen
        fertig = true;
        
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
            
            if (dataManager.deleteExperiment(filename)) {
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
            zeigeVersuchDetails(filename);
          }
        }
      }
    }
    
    // Auf Encoder-Button prüfen
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
        buttonPressed = true;
        lastDebounceTime = millis();
        
        // Zurück zur Liste der gespeicherten Versuche
        fertig = true;
        zeigeGespeicherteVersuche();
      }
    } else {
      buttonPressed = false;
    }
  }
}

/**
 * Startet einen WiFi-Hotspot zum Export der Versuchsdaten
 * Zeigt Verbindungsinformationen und URL für den Datenzugriff an
 */
void WindTurbineExperiment::zeigeWiFiExport() {
  tft.fillScreen(TFT_BACKGROUND);
  
  // Titelbereich
  zeichneTitelbalken("WiFi-Export");
  
  // Export starten
  if (dataManager.startWiFiExport(aktuellerVersuchsFilename)) {
    // Export-Informationen anzeigen
    tft.fillRoundRect(20, 50, 440, 220, 5, TFT_OUTLINE);
    
    // Überschrift
    tft.fillRect(21, 51, 438, 20, TFT_TITLE_BG);
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setTextSize(1);
    tft.setCursor(30, 56);
    tft.print("WiFi-Export aktiv");
    
    // WLAN-Informationen - IP-Adresse aus der URL extrahieren
    String exportURL = dataManager.getExportURL();
    int ipStart = exportURL.indexOf("//") + 2;
    String ipAddress = exportURL.substring(ipStart);
    
    // SSID aus dem DataManager holen
    String wifiName = dataManager.getCurrentSSID();
    
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(30, 80);
    tft.print("WLAN-Name: ");
    tft.print(wifiName);
    
    tft.setCursor(30, 100);
    tft.print("WLAN-Passwort: windturbine");
    
    tft.setCursor(30, 120);
    tft.print("URL: ");
    tft.print(exportURL);
    
    // QR-Code wurde entfernt, da er nicht zuverlässig funktioniert
    
    // Anleitung
    tft.setCursor(30, 150);
    tft.println("1. Verbinden Sie Ihr Gerät mit dem WLAN");
    tft.setCursor(30, 170);
    tft.println("2. Öffnen Sie die URL im Browser");
    tft.setCursor(30, 190);
    tft.println("3. Laden Sie die Daten im gewünschten Format herunter");
    
    // Optionen
    tft.fillRoundRect(20, 280, 440, 20, 5, TFT_SUBTITLE);
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(30, 285);
    tft.print("D=Zurück");
    
    zeichneStatusleiste("D=Zurück");
    
    // Auf Auswahl warten
    bool fertig = false;
    while (!fertig) {
      // WiFi-Export-Handler aufrufen
      dataManager.handleWiFiExport();
      
      char key = keypad.getKey();
      
      if (key) {
        if (key == 'D') {
          // Export beenden
          fertig = true;
          dataManager.stopWiFiExport();
          
          // Zurück zu den Versuchsdetails
          zeigeVersuchDetails(aktuellerVersuchsFilename);
        }
      }
      
      // Auf Encoder-Button prüfen
      if (digitalRead(ENCODER_BUTTON) == LOW) {
        if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
          buttonPressed = true;
          lastDebounceTime = millis();
          
          // Export beenden
          fertig = true;
          dataManager.stopWiFiExport();
          
          // Zurück zu den Versuchsdetails
          zeigeVersuchDetails(aktuellerVersuchsFilename);
        }
      } else {
        buttonPressed = false;
      }
    }
  } else {
    // Fehler beim Starten des Exports
    tft.fillRoundRect(90, 120, 300, 80, 8, TFT_WARNING);
    tft.setTextColor(TFT_TEXT);
    tft.setTextSize(1);
    tft.setCursor(110, 140);
    tft.println("Fehler beim Starten des WiFi-Exports!");
    tft.setCursor(110, 160);
    tft.println("Drücken Sie eine Taste, um zurückzukehren...");
    
    // Warten auf Tastendruck
    bool warten = true;
    while (warten) {
      char key = keypad.getKey();
      if (key) {
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
    
    // Zurück zu den Versuchsdetails
    zeigeVersuchDetails(aktuellerVersuchsFilename);
  }
}
