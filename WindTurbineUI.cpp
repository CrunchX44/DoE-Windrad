/**
 * WindTurbineUI.cpp
 * UI-Funktionen für das Windkraftanlagen-Experiment
 */

 #include "WindTurbineExperiment.h"

/**
 * Zeichnet einen Titelbalken am oberen Bildschirmrand
 * @param titel Text, der im Titelbalken angezeigt wird
 */
  void WindTurbineExperiment::zeichneTitelbalken(const char* titel) {
    // Titelbalken mit abgerundetem Hintergrund
    tft.fillRoundRect(10, 10, 460, HEADER_HEIGHT, 5, TFT_TITLE_BG);
    
    // Titel - Textgröße basierend auf Länge anpassen
    // Kleinere Textgröße für längere Titel verwenden
    uint8_t textSize = (strlen(titel) > 25) ? 1 : 2;
    
    tft.setTextSize(textSize);
    tft.setTextColor(TFT_HEADER);
    tft.setCursor(25, textSize == 1 ? 20 : 18);
    tft.print(titel);
    
    // Motor-Status-Box zeichnen
    zeichneMotorStatusBox();
  }
 
/**
 * Zeichnet eine Statusleiste am unteren Bildschirmrand
 * @param status Text, der in der Statusleiste angezeigt wird
 */
void WindTurbineExperiment::zeichneStatusleiste(const char* status) {
  // Statusleiste am unteren Rand
  tft.fillRect(0, 320-STATUS_BAR_HEIGHT, 480, STATUS_BAR_HEIGHT, TFT_STATUS_BAR);
  
  // Zurück-Button am rechten Rand - schmaler und weiter rechts platziert
  tft.fillRoundRect(420, 320-STATUS_BAR_HEIGHT+2, 55, STATUS_BAR_HEIGHT-4, 3, TFT_SUBTITLE);
  
  // WICHTIG: Textgröße explizit auf 1 setzen vor dem Zeichnen des Buttons
  tft.setTextSize(1);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(425, 320-12);
  tft.print("D=Zur.");
  
  // Status-Text mit dynamischer Anpassung
  // tft.setTextSize(1); // Bereits oben gesetzt
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(10, 320-15);
  
  // Textlänge berechnen
  int textLength = strlen(status);
  
  // Verfügbarer Platz für Text (abzüglich Zurück-Button und Rand)
  int maxWidth = 410;
  
  if (textLength * 6 > maxWidth) {
    // Text ist zu lang, daher kürzen
    int maxChars = maxWidth / 6;
    char gekuerzterStatus[100];
    strncpy(gekuerzterStatus, status, maxChars - 3);
    gekuerzterStatus[maxChars - 3] = '.';
    gekuerzterStatus[maxChars - 2] = '.';
    gekuerzterStatus[maxChars - 1] = '.';
    gekuerzterStatus[maxChars] = '\0';
    tft.print(gekuerzterStatus);
  } else {
    // Text passt, vollständig anzeigen
    tft.print(status);
  }
}

/**
 * Zeichnet eine kleine Box, die den Verbindungsstatus des Motors anzeigt
 * Grün = Motor verbunden, Rot = Motor nicht verbunden
 * ANGEPASST: Weiter nach links verschoben + Spannung anzeigen
 */
void WindTurbineExperiment::zeichneMotorStatusBox() {
  // Motor-Status - VERSCHOBEN: von 320 auf 250
  int boxX = 250;
  int boxY = 12;
  int boxSize = 10;
  
  // Hintergrund (schwarz für besseren Kontrast)
  tft.fillRect(boxX-1, boxY-1, boxSize+2, boxSize+2, TFT_BLACK);
  
  // Status-Box zeichnen
  if (motorStatusAktuell) {
    // Motor angeschlossen: Grüne Box
    tft.fillRect(boxX, boxY, boxSize, boxSize, TFT_SUCCESS);
  } else {
    // Motor nicht angeschlossen: Rote Box
    tft.fillRect(boxX, boxY, boxSize, boxSize, TFT_WARNING);
  }
  
  // Weißer Rand für bessere Sichtbarkeit
  tft.drawRect(boxX, boxY, boxSize, boxSize, TFT_WHITE);
  
  // Akku-Status - VERSCHOBEN: von 340 auf 270
  int battX = 270;
  int battY = 12;
  int battW = 20;
  int battH = 10;
  
  // Akku-Rahmen
  tft.drawRect(battX, battY, battW, battH, TFT_WHITE);
  tft.fillRect(battX + battW, battY + 2, 2, battH - 4, TFT_WHITE); // Plus-Pol
  
  // Akku-Füllung basierend auf Prozent
  int fuellBreite = (akkuProzent * (battW - 2)) / 100;
  uint16_t fuellFarbe;
  
  if (akkuProzent > 50) fuellFarbe = TFT_SUCCESS;
  else if (akkuProzent > 20) fuellFarbe = TFT_HIGHLIGHT;
  else fuellFarbe = TFT_WARNING;
  
  tft.fillRect(battX + 1, battY + 1, fuellBreite, battH - 2, fuellFarbe);
  
  // ERWEITERT: Prozentanzeige UND Spannung - kompakter dargestellt
  tft.setTextSize(1);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(battX + battW + 8, battY + 2);
  tft.print(akkuProzent);
  tft.print("% ");
  tft.print(akkuSpannung, 1); // Eine Nachkommastelle
  tft.print("V");
}
 
/**
 * Zeigt den Startbildschirm des Experiments an
 * Enthält Titel, Einführungstext und Optionen
 */
 void WindTurbineExperiment::zeigeIntro() {
  // Bildschirm leeren
  tft.fillScreen(TFT_BACKGROUND);

  // Hilfsfunktion: Text zentriert in Box zeichnen
  auto drawCenteredBox = [&](const char* txt, uint8_t ts, int16_t boxX, int16_t boxY, int16_t boxW, int16_t boxH, uint16_t boxCol, uint16_t txtCol) {
    tft.fillRoundRect(boxX, boxY, boxW, boxH, 5, boxCol);
    tft.setTextSize(ts);
    tft.setTextColor(txtCol);
    int16_t txtW = strlen(txt) * 6 * ts;
    int16_t txtH = 8 * ts;
    int16_t txtX = boxX + (boxW - txtW) / 2;
    int16_t txtY = boxY + (boxH - txtH) / 2; // vertikal zentriert
    tft.setCursor(txtX, txtY);
    tft.print(txt);
  };

  // --- Titel ---
  const char* title = "Windkraftanlagen-Experiment";
  const uint8_t tsTitle = 2;
  int16_t titleW = strlen(title) * 6 * tsTitle;
  int16_t titleH = 8 * tsTitle;
  int16_t titleX = (tft.width() - titleW - 20) / 2;
  int16_t titleY = 10;
  drawCenteredBox(title, tsTitle, titleX, titleY, titleW + 20, titleH + 10, TFT_TITLE_BG, TFT_HEADER);

  // --- Untertitel ---
  const char* subtitle = "Design of Experiments";
  const uint8_t tsSub = 2;
  int16_t subW = strlen(subtitle) * 6 * tsSub;
  int16_t subH = 8 * tsSub;
  int16_t subX = (tft.width() - subW - 20) / 2;
  int16_t subY = titleY + titleH + 20;
  drawCenteredBox(subtitle, tsSub, subX, subY, subW + 20, subH + 10, TFT_OUTLINE, TFT_TEXT);

  // --- Trennlinie ---
  int16_t lineY = subY + subH + 15;
  tft.drawLine(20, lineY, tft.width() - 20, lineY, TFT_GRID);

  // --- Einführungstext ---
  const char* intro = "In diesem Experiment werden Sie:";
  const uint8_t tsIntro = 1;
  tft.setTextSize(tsIntro);
  tft.setTextColor(TFT_LIGHT_TEXT);
  // Mehr Abstand unterhalb der Linie
  int16_t introY = lineY + 20;
  tft.setCursor(20, introY);
  tft.print(intro);

  // --- Schritte ---
  const char* steps[4] = {
    "Teilfaktoriellen Versuch (2^(5-2)) durchfuehren",
    "Daten auswerten und Haupteffekte berechnen",
    "Vollfaktoriellen Versuch mit 3 Faktoren durchführen",
    "Abschliessende Regression erstellen"
  };
  int16_t stepBoxSize = 16;
  // Mehr Abstand nach dem Intro-Text
  int16_t stepY = introY + 20;
  for (int i = 0; i < 4; i++) {
    int16_t boxY = stepY - (stepBoxSize / 2);
    // Zahl in Kästchen
    char numTxt[2] = { char('1' + i), '\0' };
    drawCenteredBox(numTxt, tsIntro, 20, boxY, stepBoxSize, stepBoxSize, TFT_HIGHLIGHT, TFT_BACKGROUND);
    // Beschreibung daneben (vertikal zentriert)
    tft.setTextSize(tsIntro);
    tft.setTextColor(TFT_TEXT);
    int16_t txtH = 8 * tsIntro;
    int16_t txtY = boxY + (stepBoxSize - txtH) / 2;
    tft.setCursor(40, txtY);
    tft.print(steps[i]);
    stepY += 20;
  }

  // --- Faktorenbox ---
  const char* faktorText = "Faktoren: Steigung, Groesse, Abstand, Luft, Blatt";
  const uint8_t tsFaktor = 1;
  int16_t faktW = strlen(faktorText) * 6 * tsFaktor;
  int16_t faktH = 8 * tsFaktor;
  int16_t faktX = (tft.width() - faktW - 20) / 2;
  int16_t faktY = tft.height() - 100; // Höher positioniert für Platz für Buttons
  drawCenteredBox(faktorText, tsFaktor, faktX, faktY, faktW + 20, faktH + 10, TFT_OUTLINE, TFT_SUBTITLE);

  // --- Buttons für Datenverwaltung ---
  // Button für gespeicherte Versuche
  tft.fillRoundRect(20, tft.height() - 70, 210, 30, 5, TFT_TITLE_BG);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setTextSize(1);
  tft.setCursor(30, tft.height() - 60);
  tft.print("1: Gespeicherte Versuche");

  // Button für Export
  tft.fillRoundRect(250, tft.height() - 70, 210, 30, 5, TFT_TITLE_BG);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setTextSize(1);
  tft.setCursor(260, tft.height() - 60);
  tft.print("2: Daten exportieren");

  // --- Steuerungshinweis ---
  const char* hint = "Druecken zum Starten";
  const uint8_t tsHint = 1;
  int16_t hintW = strlen(hint) * 6 * tsHint;
  int16_t hintH = 8 * tsHint;
  int16_t hintX = (tft.width() - hintW - 20) / 2;
  int16_t hintYpos = tft.height() - hintH - 35; // Höher positioniert
  drawCenteredBox(hint, tsHint, hintX, hintYpos - 5, hintW + 20, hintH + 10, TFT_SUCCESS, TFT_TEXT);

  // --- Anleitung ---
  zeichneStatusleiste("Druecken: Starten | 1: Gespeicherte Versuche | 2: Daten exportieren");

  // --- Abschluss ---
  maxCursorPosition = 0;
  aktuellerModus = INTRO;
}

 
/**
 * Zeigt den teilfaktoriellen Versuchsplan mit allen Faktoreinstellungen an
 */
 void WindTurbineExperiment::zeigeTeilfaktoriellPlan() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Teilfaktorieller Versuchsplan (2^(5-2))");
   
   // Tabellenrahmen
   tft.drawRoundRect(15, 48, 450, 190, 5, TFT_OUTLINE);
   
   // Spaltentrenner für Überschriften
   for (int i = 0; i <= 5; i++) {
     int x = 15 + i * 75;
     tft.drawLine(x, 48, x, 70, TFT_GRID);
   }
   
   // Faktoren als Spaltenüberschriften mit Hintergrund
   tft.fillRect(15, 48, 450, 22, TFT_TITLE_BG);
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   
   // Tabellenüberschriften
   tft.setCursor(25, 55);
   tft.print("Nr.");
   
   for (int i = 0; i < 5; i++) {
     tft.setCursor(55 + i * 75, 55);
     // Prüfen ob der Name zu lang ist und ggf. abkürzen
     if (strlen(faktorNamen[i]) > 9) {
       char kurzname[10];
       strncpy(kurzname, faktorNamen[i], 8);
       kurzname[8] = '.';
       kurzname[9] = '\0';
       tft.print(kurzname);
     } else {
       tft.print(faktorNamen[i]);
     }
   }
   
   // Zeilen zeichnen
   for (int i = 0; i <= 8; i++) {
     tft.drawLine(15, 70 + i * 21, 465, 70 + i * 21, TFT_GRID);
   }
   
   // Tabelleninhalt
   tft.setTextColor(TFT_TEXT);
   for (int i = 0; i < 8; i++) {
     // Versuchsnummer mit Highlight für gerade/ungerade Zeilen
     if (i % 2 == 0) {
       tft.fillRect(16, 71 + i * 21, 74, 20, 0x1082); // Leicht abgedunkelter Hintergrund
     }
     tft.setCursor(25, 80 + i * 21);
     tft.print(i + 1);
     
     // Faktorstufen
     for (int j = 0; j < 5; j++) {
       tft.setCursor(55 + j * 75, 80 + i * 21);
       if (teilfaktoriellPlan[i][j] == -1) {
         tft.setTextColor(TFT_LIGHT_TEXT);
         tft.print("-");
         tft.print(faktorEinheitenNiedrig[j]);
       } else if (teilfaktoriellPlan[i][j] == 1) {
         tft.setTextColor(TFT_HIGHLIGHT);
         tft.print("+");
         tft.print(faktorEinheitenHoch[j]);
       } else {
         tft.print("N/A");
       }
       tft.setTextColor(TFT_TEXT);
     }
   }
   
   // Legende in einem Infokasten
   tft.fillRoundRect(20, 245, 440, 40, 5, TFT_OUTLINE);
   tft.setTextColor(TFT_LIGHT_TEXT);
   tft.setCursor(30, 255);
   tft.print("-: ");
   tft.setTextColor(TFT_TEXT);
   tft.print("niedrige Stufe  ");
   tft.setTextColor(TFT_LIGHT_TEXT);
   tft.print("+: ");
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.print("hohe Stufe");
   
   // Anleitung
   zeichneStatusleiste("Druecken Sie den Drehknopf, um mit den Messungen zu beginnen.");
   
   maxCursorPosition = 0;
   aktuellerModus = TEILFAKTORIELL_PLAN;
 }
 
/**
 * Zeigt den Messbildschirm für einen teilfaktoriellen Versuch an
 * Ermöglicht die Durchführung von 5 Messungen pro Versuch
 */
 void WindTurbineExperiment::zeigeTeilfaktoriellMessung() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich mit Versuchsinfo
   char titel[50];
   sprintf(titel, "Teilfaktorieller Versuch: %d/8", aktuellerVersuch + 1);
   zeichneTitelbalken(titel);
   
   // Fortschrittsanzeige
   tft.fillRoundRect(380, 15, 90, 20, 5, TFT_TITLE_BG);
   tft.fillRect(382, 17, (aktuellerVersuch * 86) / 8, 16, TFT_HIGHLIGHT);
   
   // Bereich für Faktoreinstellungen - breiter für den Text
   tft.fillRoundRect(15, 50, 210, 140, 5, TFT_OUTLINE);
   
   // Überschrift - mit Zeilenumbruch für bessere Lesbarkeit
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(25, 60);
   tft.print("Bitte stellen Sie");
   tft.setCursor(25, 72);
   tft.print("folgende Werte ein:");
   
   // Faktoren mit visueller Hervorhebung
   tft.setTextColor(TFT_TEXT);
   for (int i = 0; i < 5; i++) {
     int y = 85 + i * 20;
     
     // Faktorname
     tft.setCursor(25, y);
     tft.print(faktorNamen[i]);
     tft.print(": ");
     
     // Wert mit farbiger Kennzeichnung
     if (teilfaktoriellPlan[aktuellerVersuch][i] == -1) {
       tft.setTextColor(TFT_LIGHT_TEXT);
       tft.print(faktorEinheitenNiedrig[i]);
       // Visuelle Kennzeichnung "niedrig"
       tft.fillRoundRect(175, y-2, 40, 15, 3, 0x1082);
       tft.setCursor(183, y);
       tft.print("(-)");
     } else if (teilfaktoriellPlan[aktuellerVersuch][i] == 1) {
       tft.setTextColor(TFT_HIGHLIGHT);
       tft.print(faktorEinheitenHoch[i]);
       // Visuelle Kennzeichnung "hoch"
       tft.fillRoundRect(175, y-2, 40, 15, 3, 0x04FF);
       tft.setCursor(183, y);
       tft.print("(+)");
     }
     tft.setTextColor(TFT_TEXT);
   }
   
   // Bereich für Messungen
   tft.fillRoundRect(235, 50, 230, 140, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(245, 60);
   tft.println("Durchgefuehrte Messungen:");
   
   // Messungen mit Fortschrittsanzeige
   tft.setTextColor(TFT_TEXT);
   for (int i = 0; i < 5; i++) {
     int y = 85 + i * 20;
     tft.setCursor(245, y);
     tft.print("Messung ");
     tft.print(i + 1);
     tft.print(": ");
     
     // Messwert oder Platzhalter
     if (i < aktuelleMessung) {
       // Leistungswert mit Messwert-Rahmen
       tft.fillRoundRect(320, y-2, 95, 15, 3, TFT_SUCCESS);
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(325, y);
       tft.print(teilfaktoriellMessungen[aktuellerVersuch][i], 2);
       tft.print(" uW");
     } else {
       // Noch nicht gemessen
       tft.drawRoundRect(320, y-2, 95, 15, 3, TFT_GRID);
       tft.setCursor(350, y);
       tft.print("---");
     }
   }
   
   // Aktuelle Ergebnisse - Mittelwert nur anzeigen, wenn er nicht später berechnet werden soll
   if (aktuelleMessung > 0) {
     tft.fillRoundRect(15, 200, 450, 60, 5, TFT_OUTLINE);
     
     // Messfortschritt immer anzeigen
     tft.setTextColor(TFT_SUBTITLE);
     tft.setCursor(25, 235);
     tft.print("Messfortschritt: ");
     
     // Balken für Messfortschritt
     tft.drawRect(170, 232, 250, 15, TFT_GRID);
     tft.fillRect(172, 234, (aktuelleMessung * 246) / 5, 11, TFT_SUCCESS);
     
     // Zahlenwert des Fortschritts
     tft.setTextColor(TFT_TEXT);
     tft.setCursor(425, 235);
     tft.print(aktuelleMessung);
     tft.print("/5");
     
     // Mittelwert anzeigen, außer bei Versuchen, wo der Benutzer später den Mittelwert berechnen soll
     if (aktuellerVersuch != 1 && aktuellerVersuch != 4 && aktuellerVersuch != 6) {
       tft.setTextColor(TFT_SUBTITLE);
       tft.setCursor(25, 210);
       tft.print("Aktueller Mittelwert: ");
       
       float summe = 0;
       for (int i = 0; i < aktuelleMessung; i++) {
         summe += teilfaktoriellMessungen[aktuellerVersuch][i];
       }
       float mittelwert = summe / aktuelleMessung;
       
       // Mittelwert mit hervorgehobenem Bereich
       tft.fillRoundRect(170, 207, 80, 18, 3, TFT_HIGHLIGHT);
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(175, 210);
       tft.print(mittelwert, 2);
       tft.print(" uW");
     }
     
   }
   
   // Keypad-Hilfe
   tft.fillRoundRect(15, 270, 450, 30, 5, TFT_SUBTITLE);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(20, 280);
   tft.println("Keypad: # = Messung wiederholen, * = Letzte Messung loeschen");
   
   // Anleitung je nach Status
   if (aktuelleMessung < 5) {
     zeichneStatusleiste("Druecken Sie den Drehknopf, um eine Messung durchzufuehren.");
   } else {
     zeichneStatusleiste("Alle Messungen abgeschlossen. Druecken zum Fortfahren.");
   }
   
   maxCursorPosition = 0;
   aktuellerModus = TEILFAKTORIELL_MESSUNG;
 }
 
/**
 * Zeigt die Auswertung des teilfaktoriellen Versuchs an
 * Stellt Mittelwerte, Standardabweichungen und Effekte dar
 */
void WindTurbineExperiment::zeigeTeilfaktoriellAuswertung() {
  tft.fillScreen(TFT_BACKGROUND);
  
  // Titelbereich
  zeichneTitelbalken("Teilfaktorieller Versuch: Ergebnisse");
  
  // Ergebnistabelle
  tft.fillRoundRect(10, 48, 230, 190, 5, TFT_OUTLINE);
  
  // Tabellenüberschrift
  tft.fillRect(11, 49, 228, 20, TFT_TITLE_BG);
  tft.setTextSize(1);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setCursor(20, 55);
  tft.print("Nr.");
  tft.setCursor(50, 55);
  tft.print("Mittelwert (uW)");
  tft.setCursor(150, 55);
  tft.print("Std.-Abw.");
  
  // Trennlinien
  tft.drawLine(45, 49, 45, 238, TFT_GRID);
  tft.drawLine(140, 49, 140, 238, TFT_GRID);
  
  // Mittelwerte und Standardabweichungen anzeigen
  tft.setTextColor(TFT_TEXT);
  for (int i = 0; i < 8; i++) {
    int y = 78 + i * 20;
    
    // Zeilenhintergrund
    if (i % 2 == 0) {
      tft.fillRect(11, y-9, 228, 19, 0x1082);
    }
    
    // Versuchsnummer
    tft.setCursor(25, y);
    tft.print(i + 1);
    
    // Mittelwert
    tft.setCursor(60, y);
    tft.print(teilfaktoriellMittelwerte[i], 2);
    
    // Standardabweichung
    tft.setCursor(150, y);
    tft.print(teilfaktoriellStandardabweichungen[i], 3);
  }
  
  // ============ KORRIGIERTE HAUPTEFFEKTE-ANZEIGE MIT MEHR PLATZ ============
  tft.fillRoundRect(245, 48, 235, 140, 5, TFT_OUTLINE); // Breiter: 220->235
  
  // Überschrift
  tft.fillRect(246, 49, 233, 20, TFT_TITLE_BG);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setCursor(320, 55);
  tft.println("Haupteffekte");
  
  // Kurze Faktornamen definieren für bessere Darstellung
  const char* kurzeFaktorNamen[] = {"Steigung", "Groesse", "Abstand", "Luftst.", "Blattanz."};
  
  // Maximalen Effektbetrag finden für relative Balkenbreite
  float maxEffektBetrag = 0;
  for (int i = 0; i < 5; i++) {
    if (abs(effekte[i]) > maxEffektBetrag) {
      maxEffektBetrag = abs(effekte[i]);
    }
  }
  
  // VERBESSERTES LAYOUT: Name (feste Breite) - Wert - Balken
  for (int i = 0; i < 5; i++) {
    int y = 80 + i * 20;
    
    // Zeilenhintergrund für bessere Lesbarkeit
    if (i % 2 == 0) {
      tft.fillRect(246, y-9, 233, 19, 0x1082);
    }
    
    // 1. Faktorname (links, feste Breite)
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(255, y);
    tft.print(kurzeFaktorNamen[i]);
    tft.print(":");
    
    // 2. Effektwert (mitte) - genug Abstand zum Namen
    tft.setCursor(330, y); // Weiter rechts verschoben: 320->330
    if (effekte[i] >= 0) {
      tft.setTextColor(TFT_SUCCESS);
      tft.print("+");
    } else {
      tft.setTextColor(TFT_WARNING);
    }
    tft.print(effekte[i], 2);
    tft.print("uW"); // Einheit hinzugefügt
    tft.setTextColor(TFT_TEXT);
    
    // 3. Balken für relative Stärke (ganz rechts)
    int balkenBreite = maxEffektBetrag > 0 ? (abs(effekte[i]) / maxEffektBetrag) * 50 : 0;
    if (balkenBreite < 3 && effekte[i] != 0) balkenBreite = 3; // Mindestbreite
    
    int balkenStart = 420; // Noch weiter rechts: 380->420
    
    if (effekte[i] > 0) {
      tft.fillRect(balkenStart, y-7, balkenBreite, 14, TFT_SUCCESS);
    } else if (effekte[i] < 0) {
      tft.fillRect(balkenStart, y-7, balkenBreite, 14, TFT_WARNING);
    }
    
    // Rahmen um Balken für bessere Sichtbarkeit
    if (effekte[i] != 0) {
      tft.drawRect(balkenStart, y-7, balkenBreite, 14, TFT_GRID);
    }
  }
  
  // Ausgewählte Faktoren
  tft.fillRoundRect(245, 198, 235, 40, 5, TFT_SUCCESS); // Angepasst an neue Breite
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(255, 208);
  tft.println("Ausgewaehlte Faktoren:");
  
  // Faktornamen
  tft.setCursor(255, 223);
  for (int i = 0; i < 3; i++) {
    if (i > 0) tft.print(", ");
    tft.print(faktorNamen[ausgewaehlteVollfaktoren[i]]);
  }
  
  // Diagramm-Optionen-Box
  tft.fillRoundRect(10, 248, 470, 50, 5, TFT_OUTLINE); // Breiter für neue Box-Breite
  
  // Optionen in einer Zeile mit visuellen Buttons
  for (int i = 0; i < 4; i++) {
    tft.fillRoundRect(25 + i*115, 258, 105, 30, 5, TFT_TITLE_BG);
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setCursor(30 + i*115, 260);
    
    switch(i) {
      case 0: tft.print("1: Balken-"); break;
      case 1: tft.print("2: Main"); break;
      case 2: tft.print("3: Inter-"); break;
      case 3: tft.print("4: Pareto-"); break;
    }
    
    tft.setCursor(30 + i*115, 273);
    switch(i) {
      case 0: tft.print("diagramm"); break;
      case 1: tft.print("Effects Plot"); break;
      case 2: tft.print("action Plot"); break;
      case 3: tft.print("diagramm"); break;
    }
  }
  
  // Anleitung
  zeichneStatusleiste("Druecken: Vollfakt. Versuch");
  
  maxCursorPosition = 0;
  aktuellerModus = TEILFAKTORIELL_AUSWERTUNG;
  // Aktuelle Versuchnummer für manuelle Berechnungen auf 0 setzen
  aktuellerVersuch = 0;
}
 
/**
 * Zeigt den vollfaktoriellen Versuchsplan mit den drei wichtigsten Faktoren an
 */
  void WindTurbineExperiment::zeigeVollfaktoriellPlan() {
    tft.fillScreen(TFT_BACKGROUND);
    
    // Titelbereich
    zeichneTitelbalken("Vollfaktorieller Versuchsplan (2^3)");
    
    // Fixierte Faktoren Sektion - kompakter
    tft.fillRoundRect(20, 48, 440, 60, 5, TFT_OUTLINE);
    tft.fillRect(21, 49, 438, 18, TFT_TITLE_BG);
    tft.setTextSize(1);
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setCursor(30, 55);
    tft.print("Fixierte Faktoren (konstant):");
    
    tft.setTextColor(TFT_TEXT);
    int yPos = 72;
    bool hatFixierteFaktoren = false;
    String fixierteText = "";
    
    for (int i = 0; i < 5; i++) {
      if (fixierteFaktorwerte[i] != 99) {
        if (hatFixierteFaktoren) fixierteText += ", ";
        fixierteText += faktorNamen[i];
        fixierteText += "=";
        if (fixierteFaktorwerte[i] == 1) {
          fixierteText += faktorEinheitenHoch[i];
        } else {
          fixierteText += faktorEinheitenNiedrig[i];
        }
        hatFixierteFaktoren = true;
      }
    }
    
    if (!hatFixierteFaktoren) {
      tft.setCursor(40, yPos);
      tft.print("Keine Faktoren fixiert");
    } else {
      tft.setCursor(40, yPos);
      tft.print(fixierteText);
      if (fixierteText.length() > 50) {
        // Text umbrechen falls zu lang
        tft.setCursor(40, yPos + 15);
        String zweiteTeil = fixierteText.substring(50);
        tft.print(zweiteTeil);
      }
    }
    
    // Vollfaktorieller Plan - korrekte Y-Position
    tft.fillRoundRect(20, 118, 440, 170, 5, TFT_OUTLINE);
    
    // Tabellenüberschrift
    tft.fillRect(21, 119, 438, 20, TFT_TITLE_BG);
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setCursor(30, 125);
    tft.print("Nr.");
    
    for (int i = 0; i < 3; i++) {
      int faktorIndex = ausgewaehlteVollfaktoren[i];
      tft.setCursor(70 + i * 120, 125);
      // Faktorname kürzen falls nötig
      if (strlen(faktorNamen[faktorIndex]) > 9) {
        char kurzname[10];
        strncpy(kurzname, faktorNamen[faktorIndex], 8);
        kurzname[8] = '.';
        kurzname[9] = '\0';
        tft.print(kurzname);
      } else {
        tft.print(faktorNamen[faktorIndex]);
      }
    }
    
    // Vollfaktoriellen Plan erstellen
    int vollfaktoriellPlan[8][5];
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 5; j++) {
        if (fixierteFaktorwerte[j] != 99) {
          vollfaktoriellPlan[i][j] = fixierteFaktorwerte[j];
        } else {
          vollfaktoriellPlan[i][j] = 0;
        }
      }
    }
    
    for (int i = 0; i < 8; i++) {
      vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[0]] = (i & 1) ? 1 : -1;
      vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[1]] = (i & 2) ? 1 : -1;
      vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[2]] = (i & 4) ? 1 : -1;
    }
    
    // Trennlinien - korrekte Positionen
    for (int i = 0; i <= 8; i++) {
      tft.drawLine(21, 139 + i * 18, 458, 139 + i * 18, TFT_GRID);
    }
    
    tft.drawLine(60, 119, 60, 283, TFT_GRID);
    tft.drawLine(180, 119, 180, 283, TFT_GRID);
    tft.drawLine(300, 119, 300, 283, TFT_GRID);
    
    // Tabelleninhalt - korrekte Y-Positionen
    tft.setTextColor(TFT_TEXT);
    for (int i = 0; i < 8; i++) {
      int y = 150 + i * 18;
      
      if (i % 2 == 0) {
        tft.fillRect(21, y-8, 438, 17, 0x1082);
      }
      
      // Versuchsnummer
      tft.setCursor(38, y);
      tft.print(i + 1);
      
      // Faktorstufen
      for (int j = 0; j < 3; j++) {
        int faktorIndex = ausgewaehlteVollfaktoren[j];
        int x = 70 + j * 120;
        
        if (vollfaktoriellPlan[i][faktorIndex] == -1) {
          tft.fillRoundRect(x, y-6, 12, 12, 3, 0x1082);
          tft.setTextColor(TFT_LIGHT_TEXT);
          tft.setCursor(x+3, y);
          tft.print("-");
          
          tft.setTextColor(TFT_TEXT);
          tft.setCursor(x+15, y);
          tft.print(faktorEinheitenNiedrig[faktorIndex]);
        } else if (vollfaktoriellPlan[i][faktorIndex] == 1) {
          tft.fillRoundRect(x, y-6, 12, 12, 3, 0x04FF);
          tft.setTextColor(TFT_HIGHLIGHT);
          tft.setCursor(x+3, y);
          tft.print("+");
          
          tft.setTextColor(TFT_TEXT);
          tft.setCursor(x+15, y);
          tft.print(faktorEinheitenHoch[faktorIndex]);
        }
      }
    }
    
    // Anleitung
    zeichneStatusleiste("Druecken Sie den Drehknopf, um mit den Messungen zu beginnen.");
    
    maxCursorPosition = 0;
    aktuellerModus = VOLLFAKTORIELL_PLAN;
  }
 
/**
 * Zeigt den Messbildschirm für einen vollfaktoriellen Versuch an
 * Ermöglicht die Durchführung von 5 Messungen pro Versuch
 */
  void WindTurbineExperiment::zeigeVollfaktoriellMessung() {
    tft.fillScreen(TFT_BACKGROUND);
    
    // Titelbereich mit Versuchsinfo
    char titel[50];
    sprintf(titel, "Vollfaktorieller Versuch: %d/8", aktuellerVersuch + 1);
    zeichneTitelbalken(titel);
    
    // Fortschrittsanzeige
    tft.fillRoundRect(380, 15, 90, 20, 5, TFT_TITLE_BG);
    tft.fillRect(382, 17, (aktuellerVersuch * 86) / 8, 16, TFT_HIGHLIGHT);
    
    // Plan erstellen
    int vollfaktoriellPlan[8][5];
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 5; j++) {
        if (fixierteFaktorwerte[j] != 99) {
          vollfaktoriellPlan[i][j] = fixierteFaktorwerte[j];
        } else {
          vollfaktoriellPlan[i][j] = 0;
        }
      }
    }
    
    for (int i = 0; i < 8; i++) {
      vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[0]] = (i & 1) ? 1 : -1;
      vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[1]] = (i & 2) ? 1 : -1;
      vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[2]] = (i & 4) ? 1 : -1;
    }
    
    // Faktoreinstellungen - erweitert für alle 5 Faktoren
    tft.fillRoundRect(15, 50, 230, 150, 5, TFT_OUTLINE);
    
    tft.setTextSize(1);
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setCursor(25, 60);
    tft.print("Alle Faktoreinstellungen:");
    
    tft.setTextColor(TFT_TEXT);
    for (int i = 0; i < 5; i++) {
      int y = 75 + i * 22;
      
      // Faktorname - gekürzt
      tft.setCursor(25, y);
      if (strlen(faktorNamen[i]) > 8) {
        char kurzname[9];
        strncpy(kurzname, faktorNamen[i], 7);
        kurzname[7] = '.';
        kurzname[8] = '\0';
        tft.print(kurzname);
      } else {
        tft.print(faktorNamen[i]);
      }
      tft.print(": ");
      
      // Korrigierte Positionen für Symbole und Text
      if (fixierteFaktorwerte[i] != 99) {
        // Fixierter Faktor
        if (fixierteFaktorwerte[i] == -1) {
          tft.fillRoundRect(95, y-3, 15, 12, 3, 0x1082);
          tft.setTextColor(TFT_LIGHT_TEXT);
          tft.setCursor(98, y);
          tft.print("-");
          
          tft.setTextColor(TFT_TEXT);
          tft.setCursor(115, y);
          tft.print(faktorEinheitenNiedrig[i]);
          
          tft.setTextColor(TFT_LIGHT_TEXT);
          tft.setCursor(180, y);
          tft.print("(fix)");
        } else {
          tft.fillRoundRect(95, y-3, 15, 12, 3, 0x04FF);
          tft.setTextColor(TFT_SUCCESS);
          tft.setCursor(98, y);
          tft.print("+");
          
          tft.setTextColor(TFT_TEXT);
          tft.setCursor(115, y);
          tft.print(faktorEinheitenHoch[i]);
          
          tft.setTextColor(TFT_LIGHT_TEXT);
          tft.setCursor(180, y);
          tft.print("(fix)");
        }
      } else {
        // Variabler Faktor
        if (vollfaktoriellPlan[aktuellerVersuch][i] == -1) {
          tft.fillRoundRect(95, y-3, 15, 12, 3, 0x1082);
          tft.setTextColor(TFT_LIGHT_TEXT);
          tft.setCursor(98, y);
          tft.print("-");
          
          tft.setTextColor(TFT_TEXT);
          tft.setCursor(115, y);
          tft.print(faktorEinheitenNiedrig[i]);
        } else if (vollfaktoriellPlan[aktuellerVersuch][i] == 1) {
          tft.fillRoundRect(95, y-3, 15, 12, 3, 0x04FF);
          tft.setTextColor(TFT_HIGHLIGHT);
          tft.setCursor(98, y);
          tft.print("+");
          
          tft.setTextColor(TFT_TEXT);
          tft.setCursor(115, y);
          tft.print(faktorEinheitenHoch[i]);
        }
      }
      tft.setTextColor(TFT_TEXT);
    }
    
    // Messungen - Position angepasst
    tft.fillRoundRect(255, 50, 210, 150, 5, TFT_OUTLINE);
    
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.setCursor(265, 60);
    tft.println("Durchgefuehrte Messungen:");
    
    tft.setTextColor(TFT_TEXT);
    for (int i = 0; i < 5; i++) {
      int y = 85 + i * 20;
      tft.setCursor(265, y);
      tft.print("Messung ");
      tft.print(i + 1);
      tft.print(": ");
      
      if (i < aktuelleMessung) {
        tft.fillRoundRect(340, y-2, 95, 15, 3, TFT_SUCCESS);
        tft.setTextColor(TFT_TEXT);
        tft.setCursor(345, y);
        tft.print(vollfaktoriellMessungen[aktuellerVersuch][i], 2);
        tft.print(" uW");
      } else {
        tft.drawRoundRect(340, y-2, 95, 15, 3, TFT_GRID);
        tft.setCursor(370, y);
        tft.print("---");
      }
    }
    
    // Rest der Funktion bleibt unverändert...
    // (Aktuelle Ergebnisse, Keypad-Hilfe, Anleitung)
    
    if (aktuelleMessung > 0) {
      tft.fillRoundRect(15, 210, 450, 60, 5, TFT_OUTLINE);
      
      tft.setTextColor(TFT_SUBTITLE);
      tft.setCursor(25, 245);
      tft.print("Messfortschritt: ");
      
      tft.drawRect(170, 242, 250, 15, TFT_GRID);
      tft.fillRect(172, 244, (aktuelleMessung * 246) / 5, 11, TFT_SUCCESS);
      
      tft.setTextColor(TFT_TEXT);
      tft.setCursor(425, 245);
      tft.print(aktuelleMessung);
      tft.print("/5");
      
      if (aktuelleMessung == 5 && aktuellerVersuch != 2 && aktuellerVersuch != 5) {
        tft.setTextColor(TFT_SUBTITLE);
        tft.setCursor(25, 220);
        tft.print("Aktueller Mittelwert: ");
        
        float summe = 0;
        for (int i = 0; i < aktuelleMessung; i++) {
          summe += vollfaktoriellMessungen[aktuellerVersuch][i];
        }
        float mittelwert = summe / aktuelleMessung;
        
        tft.fillRoundRect(170, 217, 80, 18, 3, TFT_HIGHLIGHT);
        tft.setTextColor(TFT_TEXT);
        tft.setCursor(175, 220);
        tft.print(mittelwert, 2);
        tft.print(" uW");
      }
    }
    
    tft.fillRoundRect(15, 280, 450, 30, 5, TFT_SUBTITLE);
    tft.setTextColor(TFT_TEXT);
    tft.setCursor(20, 290);
    tft.println("Keypad: # = Messung wiederholen, * = Letzte Messung loeschen");
    
    if (aktuelleMessung < 5) {
      zeichneStatusleiste("Druecken Sie den Drehknopf, um eine Messung durchzufuehren.");
    } else {
      zeichneStatusleiste("Alle Messungen abgeschlossen. Druecken zum Fortfahren.");
    }
    
    maxCursorPosition = 0;
    aktuellerModus = VOLLFAKTORIELL_MESSUNG;
  }
 
/**
 * Zeigt die Auswertung des vollfaktoriellen Versuchs an
 * Stellt Mittelwerte, Standardabweichungen und Faktoreinstellungen dar
 */
 void WindTurbineExperiment::zeigeVollfaktoriellAuswertung() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Vollfaktorieller Versuch: Ergebnisse");
   
   // Ergebnistabelle - Höhe reduziert um die 8. Zeile anzupassen
   tft.fillRoundRect(10, 48, 460, 150, 5, TFT_OUTLINE);
   
   // Tabellenüberschrift
   tft.fillRect(11, 49, 458, 20, TFT_TITLE_BG);
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(20, 55);
   tft.print("Nr.");
   tft.setCursor(50, 55);
  tft.print("Mittelwert (uW)");
   tft.setCursor(170, 55);
   tft.print("Std.-Abw.");
   tft.setCursor(280, 55);
   tft.print("Faktoreinstellungen");
   
   // Trennlinien
   tft.drawLine(45, 49, 45, 197, TFT_GRID);
   tft.drawLine(140, 49, 140, 197, TFT_GRID);
   tft.drawLine(260, 49, 260, 197, TFT_GRID);
   
   // Vollfaktorieller Plan erstellen
   int vollfaktoriellPlan[8][5];
   for (int i = 0; i < 8; i++) {
     for (int j = 0; j < 5; j++) {
       vollfaktoriellPlan[i][j] = 0;
     }
   }
   
   for (int i = 0; i < 8; i++) {
     vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[0]] = (i & 1) ? 1 : -1;
     vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[1]] = (i & 2) ? 1 : -1;
     vollfaktoriellPlan[i][ausgewaehlteVollfaktoren[2]] = (i & 4) ? 1 : -1;
   }
   
   // Mittelwerte und Standardabweichungen anzeigen - kompakter Layout
   tft.setTextColor(TFT_TEXT);
   for (int i = 0; i < 8; i++) {
     int y = 80 + i * 17; // Reduzierte Zeilenhöhe von 20 auf 17
     
     // Zeilenhintergrund
     if (i % 2 == 0) {
       tft.fillRect(11, y-8, 458, 16, 0x1082);
     }
     
     // Versuchsnummer
     tft.setCursor(25, y);
     tft.print(i + 1);
     
     // Mittelwert
     tft.setCursor(60, y);
     tft.print(vollfaktoriellMittelwerte[i], 2);
     
     // Standardabweichung
     tft.setCursor(170, y);
     tft.print(vollfaktoriellStandardabweichungen[i], 3);
     
     // Faktoreinstellungen visualisieren
     int xOffset = 280;
     for (int j = 0; j < 3; j++) {
       int faktorIndex = ausgewaehlteVollfaktoren[j];
       
       // Farbkodierte Box für die Faktoreinstellung
       if (vollfaktoriellPlan[i][faktorIndex] == -1) {
         tft.fillRoundRect(xOffset, y-5, 20, 12, 3, 0x1082);
         tft.setTextColor(TFT_LIGHT_TEXT);
       } else {
         tft.fillRoundRect(xOffset, y-5, 20, 12, 3, 0x04FF);
         tft.setTextColor(TFT_HIGHLIGHT);
       }
       
       // Symbol für die Stufe
       tft.setCursor(xOffset+6, y);
       tft.print(vollfaktoriellPlan[i][faktorIndex] == -1 ? "-" : "+");
       tft.setTextColor(TFT_TEXT);
       
       // Nächste Position
       xOffset += 50;
     }
   }
   
   // Legende für Faktoreinstellungen - Position angepasst
   tft.fillRoundRect(10, 208, 460, 20, 5, TFT_SUBTITLE);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(20, 212);
   tft.print("Faktoren: ");
   
   int xOffset = 80;
   for (int i = 0; i < 3; i++) {
     int faktorIndex = ausgewaehlteVollfaktoren[i];
     tft.setCursor(xOffset, 212);
     tft.print(i+1);
     tft.print("=");
     tft.print(faktorNamen[faktorIndex]);
     
     xOffset += 120;
   }
   
   // Diagramm-Button - Position angepasst
   tft.fillRoundRect(10, 235, 220, 30, 5, TFT_TITLE_BG);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(15, 245);
   tft.print("1 = Ergebnis-Diagramm");
   
   // Weiter-Button - Position angepasst
   tft.fillRoundRect(240, 235, 230, 30, 5, TFT_SUCCESS);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(255, 245);
   tft.print("Weiter zum Regressionsmodell");
   
   // Anleitung
   zeichneStatusleiste("Druecken: Regression");
   
   maxCursorPosition = 0;
   aktuellerModus = VOLLFAKTORIELL_AUSWERTUNG;
   // Aktuelle Versuchnummer für manuelle Berechnungen auf 0 setzen
   aktuellerVersuch = 0;
 }
 
/**
 * Zeigt das Regressionsmodell mit Koeffizienten und Modellqualität an
 */
 void WindTurbineExperiment::zeigeRegressionModell() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Regressionsmodell");
   
   // Hauptbereich für Modell
   tft.fillRoundRect(20, 50, 440, 90, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(30, 60);
   tft.println("Lineares Modell:");
   
   // Modellgleichung
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 80);
   tft.print("Leistung = b0");
   
   for (int i = 0; i < 3; i++) {
     int faktorIndex = ausgewaehlteVollfaktoren[i];
     tft.print(" + b");
     tft.print(i + 1);
     tft.print(" * ");
     tft.print(faktorNamen[faktorIndex]);
   }
   
   // Koeffizienten-Bereich
   tft.fillRoundRect(20, 150, 220, 120, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(30, 160);
   tft.println("Regressionskoeffizienten:");
   
   // Koeffizienten mit farbigen Anzeigen
   tft.setTextColor(TFT_TEXT);
   
   // Konstantterm
   float b0 = berechneRegressionsKoeffizient(0);
   tft.setCursor(30, 180);
   tft.print("b0 = ");
   
   // Wert mit Box
   tft.fillRoundRect(70, 177, 55, 15, 3, TFT_TITLE_BG);
   tft.setCursor(75, 180);
   tft.print(b0, 2);
   
   // Faktorkoeffizienten
   for (int i = 0; i < 3; i++) {
     int faktorIndex = ausgewaehlteVollfaktoren[i];
     float bi = berechneRegressionsKoeffizient(i + 1);
     int y = 200 + i * 20;
     
     // Faktorname
     tft.setCursor(30, y);
     tft.print("b");
     tft.print(i + 1);
     tft.print(" (");
     tft.print(faktorNamen[faktorIndex]);
     tft.print(") = ");
     
     // Koeffizient mit farbiger Anzeige je nach Vorzeichen
     if (bi > 0) {
       tft.fillRoundRect(140, y-3, 55, 15, 3, TFT_SUCCESS);
     } else {
       tft.fillRoundRect(140, y-3, 55, 15, 3, TFT_WARNING);
     }
     
     tft.setCursor(145, y);
     tft.print(bi, 2);
   }
   
   // Modellqualität-Bereich
   tft.fillRoundRect(250, 150, 210, 45, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(260, 160);
   tft.println("Modellqualitaet:");
   
   // R²-Wert
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(260, 180);
   tft.print("R^2 = ");
   
   float r2 = berechneR2();
   
   // R² farbig anzeigen nach Qualität
   if (r2 > 0.8) {
     tft.fillRoundRect(310, 177, 55, 15, 3, TFT_SUCCESS);
   } else if (r2 > 0.5) {
     tft.fillRoundRect(310, 177, 55, 15, 3, TFT_HIGHLIGHT);
   } else {
     tft.fillRoundRect(310, 177, 55, 15, 3, TFT_WARNING);
   }
   
   tft.setCursor(315, 180);
   tft.print(r2, 3);
   
   // Optimale Einstellungen - vergrößert für perfekte Passung
   tft.fillRoundRect(250, 205, 210, 85, 5, TFT_SUCCESS);
   
   // Überschrift
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(260, 215);
   tft.println("Optimale Einstellungen:");
   
   // Faktoren und Empfehlungen - mehr Platz zwischen den Zeilen
   for (int i = 0; i < 3; i++) {
     int faktorIndex = ausgewaehlteVollfaktoren[i];
     float koeff = berechneRegressionsKoeffizient(i + 1);
     int y = 235 + i * 20; // Mehr Abstand zwischen den Zeilen
     
     // Faktorname - gekürzt wenn zu lang
     tft.setCursor(260, y);
     if (strlen(faktorNamen[faktorIndex]) > 7) {
       char kurzname[8];
       strncpy(kurzname, faktorNamen[faktorIndex], 6);
       kurzname[6] = '.';
       kurzname[7] = '\0';
       tft.print(kurzname);
     } else {
       tft.print(faktorNamen[faktorIndex]);
     }
     tft.print(": ");
     
     // Empfohlener Wert mit Rahmen - angepasste Breite für bessere Passung
     if (koeff > 0) {
       tft.fillRoundRect(330, y-3, 120, 15, 3, 0x04FF);
       tft.setCursor(335, y);
       tft.print(faktorEinheitenHoch[faktorIndex]);
       tft.print(" (+)");
     } else {
       tft.fillRoundRect(330, y-3, 120, 15, 3, 0x1082);
       tft.setCursor(335, y);
       tft.print(faktorEinheitenNiedrig[faktorIndex]);
       tft.print(" (-)");
     }
   }
   
   // Anleitung
   zeichneStatusleiste("Druecken Sie den Drehknopf, um die Zusammenfassung anzuzeigen.");
   
   maxCursorPosition = 0;
   aktuellerModus = REGRESSION;
 }
 
/**
 * Zeigt eine Zusammenfassung des gesamten Experiments an
 * Enthält Faktoren-Rangliste und optimale Einstellungen
 */
 void WindTurbineExperiment::zeigeZusammenfassung() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Zusammenfassung des Experiments");
   
   // Faktoren-Rangliste
   tft.fillRoundRect(10, 50, 300, 170, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.fillRect(11, 51, 298, 20, TFT_TITLE_BG);
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(20, 56);
   tft.println("Faktoren nach Effektstaerke geordnet:");
   
   // Sortierte Effekte
   int sortierteFaktoren[5];
   float sortierteEffekte[5];
   
   // Kopieren der Effekte
   for (int i = 0; i < 5; i++) {
     sortierteFaktoren[i] = i;
     sortierteEffekte[i] = abs(effekte[i]); // Absolutwert für die Sortierung
   }
   
   // Einfache Bubble-Sort
   for (int i = 0; i < 4; i++) {
     for (int j = 0; j < 4 - i; j++) {
       if (sortierteEffekte[j] < sortierteEffekte[j + 1]) {
         // Effekte tauschen
         float tempEffekt = sortierteEffekte[j];
         sortierteEffekte[j] = sortierteEffekte[j + 1];
         sortierteEffekte[j + 1] = tempEffekt;
         
         // Indizes tauschen
         int tempIndex = sortierteFaktoren[j];
         sortierteFaktoren[j] = sortierteFaktoren[j + 1];
         sortierteFaktoren[j + 1] = tempIndex;
       }
     }
   }
   
   // Maximalwert für relative Balken
   float maxEffekt = sortierteEffekte[0];
   
   // Effekte anzeigen
   for (int i = 0; i < 5; i++) {
     int y = 85 + i * 25;
     int faktorIndex = sortierteFaktoren[i];
     
     // Zeilenhintergrund
     if (i % 2 == 0) {
       tft.fillRect(11, y-10, 298, 20, 0x1082);
     }
     
     // Rangplatz
     tft.setTextColor(TFT_TEXT);
     tft.setCursor(20, y);
     tft.print(i + 1);
     tft.print(". ");
     
     // Faktorname
     tft.setCursor(40, y);
     tft.print(faktorNamen[faktorIndex]);
     
     // Effektwert mit Balken für relative Stärke
     int balkenBreite = (sortierteEffekte[i] / maxEffekt) * 100;
     if (balkenBreite < 5 && sortierteEffekte[i] > 0) balkenBreite = 5; // Mindestbreite
     
     if (effekte[faktorIndex] > 0) {
       tft.fillRect(170, y-7, balkenBreite, 14, TFT_SUCCESS);
     } else if (effekte[faktorIndex] < 0) {
       tft.fillRect(170, y-7, balkenBreite, 14, TFT_WARNING);
     }
     
     // Effektwert
     tft.setCursor(280, y);
     tft.print(effekte[faktorIndex], 2);
   }
   
   // Optimale Einstellungen
   tft.fillRoundRect(320, 50, 150, 170, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.fillRect(321, 51, 148, 20, TFT_TITLE_BG);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(330, 56);
   tft.println("Optimale Einstellungen:");
   
   // Faktoren und optimale Werte
   for (int i = 0; i < 5; i++) {
     int faktorIndex = i;
     int y = 85 + i * 25;
     
     // Faktorname
     tft.setTextColor(TFT_TEXT);
     tft.setCursor(330, y);
     tft.print(faktorNamen[faktorIndex]);
     tft.print(": ");
     
     // Optimaler Wert mit farbiger Kennzeichnung
     if (effekte[faktorIndex] > 0) {
       // Hohe Stufe ist optimal
       tft.fillRoundRect(420, y-7, 15, 15, 3, 0x04FF);
       tft.setTextColor(TFT_HIGHLIGHT);
       tft.setCursor(424, y);
       tft.print("+");
       
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(440, y);
       tft.print(faktorEinheitenHoch[faktorIndex]);
     } else {
       // Niedrige Stufe ist optimal
       tft.fillRoundRect(420, y-7, 15, 15, 3, 0x1082);
       tft.setTextColor(TFT_LIGHT_TEXT);
       tft.setCursor(424, y);
       tft.print("-");
       
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(440, y);
       tft.print(faktorEinheitenNiedrig[faktorIndex]);
     }
   }
   
   // Prognose - Fixierung der doppelten Anzeige
   tft.fillRoundRect(10, 230, 460, 40, 5, TFT_SUCCESS);
   
   // Überschrift
   tft.setTextColor(TFT_TEXT);
  
   tft.setCursor(20, 240);
   tft.println("Vorhergesagte maximale Leistung bei optimalen Einstellungen:");
   
   // Wert - nur einmal anzeigen
   float prognose = berechnePrognose();
   tft.setTextSize(2);
   tft.setCursor(150, 245);
   tft.print(prognose, 2);
   tft.print(" uW");
   
   // Anleitung - Automatisches Speichern und Zurück zum Start
   zeichneStatusleiste("Druecken: Zum Startbildschirm zurueckkehren");
   
   // Automatisches Speichern des Versuchs
   char autoBeschreibung[100];
   sprintf(autoBeschreibung, "Versuch %d", anzahlGespeicherteVersuche + 1);
   
   // Speichern mit Fehlerbehandlung
   bool saveSuccess = dataManager.saveExperiment(autoBeschreibung, teilfaktoriellMessungen, 
                            teilfaktoriellMittelwerte, teilfaktoriellStandardabweichungen,
                            vollfaktoriellMessungen, vollfaktoriellMittelwerte, 
                            vollfaktoriellStandardabweichungen, effekte, 
                            ausgewaehlteVollfaktoren);
   
   // Speicherstatus anzeigen - nur kurz
   if (saveSuccess) {
     // Erfolgsmeldung kurz anzeigen
     tft.fillRoundRect(120, 280, 240, 30, 5, TFT_SUCCESS);
     tft.setTextColor(TFT_TEXT);
     tft.setTextSize(1);
     tft.setCursor(130, 290);
     tft.print("Versuch erfolgreich gespeichert");
     
     // Kurze Verzögerung, dann Meldung löschen
     delay(1500);
     
     // Meldung übermalen mit Hintergrundfarbe
     tft.fillRoundRect(120, 280, 240, 30, 5, TFT_BACKGROUND);
   } else {
     // Fehlermeldung kurz anzeigen
     tft.fillRoundRect(120, 280, 240, 30, 5, TFT_WARNING);
     tft.setTextColor(TFT_TEXT);
     tft.setTextSize(1);
     tft.setCursor(130, 290);
     tft.print("Fehler beim Speichern!");
     
     // Kurze Verzögerung, dann Meldung löschen
     delay(1500);
     
     // Meldung übermalen mit Hintergrundfarbe
     tft.fillRoundRect(120, 280, 240, 30, 5, TFT_BACKGROUND);
   }
   
   maxCursorPosition = 0;
   aktuellerModus = ZUSAMMENFASSUNG;
 }
 
 // Moderne Feedback-Anzeige Hilfsfunktion
 void WindTurbineExperiment::zeigeFeedback(bool korrekt, float eingabe, float korrekterWert, const char* einheit, const char* kategorie) {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Moderner Titelbereich mit Farbverlauf
   if (korrekt) {
     // Erfolg - Grün-Verlauf
     for (int i = 0; i < 40; i++) {
       uint16_t color = tft.color565(0, 100 + i*2, 20 + i);
       tft.drawFastHLine(0, 20 + i, 480, color);
     }
   } else {
     // Fehler - Orange-Verlauf
     for (int i = 0; i < 40; i++) {
       uint16_t color = tft.color565(150 + i, 80 + i/2, 0);
       tft.drawFastHLine(0, 20 + i, 480, color);
     }
   }
   
   // Hauptbereich mit abgerundeten Ecken und Schatten
   uint16_t hauptFarbe = korrekt ? TFT_SUCCESS : 0xFD20;
   
   // Schatteneffekt
   tft.fillRoundRect(42, 82, 396, 156, 12, 0x2104);
   // Hauptbereich
   tft.fillRoundRect(40, 80, 396, 156, 12, hauptFarbe);
   tft.fillRoundRect(45, 85, 386, 146, 10, TFT_BACKGROUND);
   
   // Icon-Bereich
   tft.fillCircle(120, 140, 35, hauptFarbe);
   tft.fillCircle(120, 140, 30, TFT_BACKGROUND);
   
   if (korrekt) {
     // Checkmark
     tft.drawLine(110, 140, 115, 145, TFT_SUCCESS);
     tft.drawLine(115, 145, 130, 130, TFT_SUCCESS);
     tft.drawLine(111, 140, 116, 145, TFT_SUCCESS);
     tft.drawLine(116, 145, 131, 130, TFT_SUCCESS);
     tft.drawLine(112, 140, 117, 145, TFT_SUCCESS);
     tft.drawLine(117, 145, 132, 130, TFT_SUCCESS);
   } else {
     // X-Mark
     tft.drawLine(110, 130, 130, 150, 0xFD20);
     tft.drawLine(130, 130, 110, 150, 0xFD20);
     tft.drawLine(111, 130, 131, 150, 0xFD20);
     tft.drawLine(131, 130, 111, 150, 0xFD20);
     tft.drawLine(109, 130, 129, 150, 0xFD20);
     tft.drawLine(129, 130, 109, 150, 0xFD20);
   }
   
   // Titel
   tft.setTextSize(2);
   tft.setTextColor(hauptFarbe);
   tft.setCursor(170, 110);
   if (korrekt) {
     tft.print("Exzellent!");
   } else {
     tft.print("Fast richtig!");
   }
   
   // Untertitel
   tft.setTextSize(1);
   tft.setTextColor(TFT_SUBTITLE);
   tft.setCursor(170, 130);
   tft.print(kategorie);
   tft.print("-Berechnung");
   
   // Ergebnis-Vergleich in modernen Boxen
   tft.fillRoundRect(50, 170, 180, 50, 8, TFT_OUTLINE);
   tft.fillRoundRect(250, 170, 180, 50, 8, TFT_OUTLINE);
   
   // Ihre Eingabe
   tft.fillRoundRect(52, 172, 176, 20, 6, TFT_TITLE_BG);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(60, 178);
   tft.print("Ihre Eingabe:");
   
   tft.setTextSize(2);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(70, 195);
   tft.print(eingabe, korrekt ? 2 : 3);
   tft.print(" ");
   tft.print(einheit);
   
   // Korrekter Wert
   tft.fillRoundRect(252, 172, 176, 20, 6, TFT_TITLE_BG);
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(260, 178);
   tft.print("Korrekter Wert:");
   
   tft.setTextSize(2);
   tft.setTextColor(korrekt ? TFT_SUCCESS : 0xFD20);
   tft.setCursor(270, 195);
   tft.print(korrekterWert, korrekt ? 2 : 3);
   tft.print(" ");
   tft.print(einheit);
   
   // Fortsetzungs-Button
   tft.fillRoundRect(150, 240, 180, 50, 10, TFT_HIGHLIGHT);
   tft.fillRoundRect(155, 245, 170, 40, 8, TFT_BACKGROUND);
   
   // Button-Icon (Pfeil)
   tft.fillTriangle(175, 260, 175, 275, 185, 267, TFT_HIGHLIGHT);
   
   // Button-Text
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(200, 258);
   tft.print("Weiter");
   tft.setCursor(200, 270);
   tft.print("Drehknopf druecken");
   
   // Motivations-Text
   if (korrekt) {
     tft.setTextColor(TFT_SUCCESS);
     tft.setCursor(60, 310);
     tft.print("Perfekt! Sie haben den Wert korrekt berechnet.");
   } else {
     tft.setTextColor(TFT_TEXT);
     tft.setCursor(60, 310);
     tft.print("Kein Problem! Uebung macht den Meister.");
   }
 }
 
 // Funktion für manuelle Eingabe von Mittelwerten durch Studenten
 void WindTurbineExperiment::manuelleMittelwertEingabe(bool istTeilfaktoriell, int versuchIndex, bool zurueckZurAuswertung) {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Mittelwert-Berechnung");
   
   // Versuchsinfo
   tft.fillRoundRect(20, 50, 440, 30, 5, TFT_OUTLINE);
   tft.setTextSize(1);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 60);
   tft.print("Versuch: ");
   tft.print(versuchIndex + 1);
   tft.print(istTeilfaktoriell ? " (Teilfaktoriell)" : " (Vollfaktoriell)");
   
   // Messwertebereich
   tft.fillRoundRect(20, 90, 440, 120, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(30, 100);
  tft.println("Messwerte (uW):");
   
   // Messungen anzeigen
   tft.setTextColor(TFT_TEXT);
   float* messungen;
   if (istTeilfaktoriell) {
     messungen = teilfaktoriellMessungen[versuchIndex];
   } else {
     messungen = vollfaktoriellMessungen[versuchIndex];
   }
   
   // Messwerte als Tabelle
   for (int i = 0; i < 5; i++) {
     int y = 120 + i * 17;
     
     // Zellhintergrund
     if (i % 2 == 0) {
       tft.fillRect(30, y-7, 420, 15, 0x1082);
     }
     
     // Messungsnummer
     tft.setCursor(40, y);
     tft.print("Messung ");
     tft.print(i+1);
     tft.print(": ");
     
     // Messwert
     tft.fillRoundRect(150, y-7, 80, 15, 3, TFT_TITLE_BG);
     tft.setCursor(160, y);
     tft.print(messungen[i], 2);
     tft.print(" uW");
   }
   
   // Aufforderungsbereich
   tft.fillRoundRect(20, 220, 440, 40, 5, TFT_SUCCESS);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 230);
   tft.println("Berechnen Sie den Mittelwert und geben Sie ihn ein:");
   
   // Eingabefeld
   tft.fillRoundRect(30, 270, 280, 40, 5, TFT_OUTLINE);
   tft.drawRect(32, 272, 276, 36, TFT_HIGHLIGHT);
   
   // Hilfstext am Eingabefeld
   tft.setCursor(35, 292);
   tft.setTextColor(TFT_LIGHT_TEXT);
   tft.print("Wert eingeben...");
   
   // Hilfetext - mit Löschfunktion, größere Box
   tft.fillRoundRect(320, 270, 140, 50, 5, TFT_SUBTITLE);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(330, 275);
   tft.println("* = Dezimalpunkt");
   tft.setCursor(330, 290);
   tft.println("A = Loeschen");
   tft.setCursor(330, 305);
   tft.println("# = Bestaetigen");
   
   // Variablen für die Eingabe
   String eingabe = "";
   bool fertig = false;
   
   // Eingabeschleife
   while (!fertig) {
     char key = keypad.getKey();
     
     if (key) {
       if (key >= '0' && key <= '9') {
         // Ziffer hinzufügen
         eingabe += key;
       } else if (key == '*' && eingabe.indexOf('.') == -1) {
         // Dezimalpunkt hinzufügen (nur einmal)
         eingabe += '.';
       } else if (key == '#') {
         // Eingabe bestätigen
         fertig = true;
       } else if (key == 'A' && eingabe.length() > 0) {
         // Letztes Zeichen löschen
         eingabe = eingabe.substring(0, eingabe.length() - 1);
       } else if (key == 'D') {
         // Zurück-Taste - Eingabe abbrechen und zurück zum korrekten Ort
         fertig = true;
         if (zurueckZurAuswertung) {
           // Zurück zur Auswertung
           if (istTeilfaktoriell) {
             zeigeTeilfaktoriellAuswertung();
           } else {
             zeigeVollfaktoriellAuswertung();
           }
         } else {
           // Zurück zur Messung des korrekten Versuchs
           aktuellerVersuch = versuchIndex;
           aktuelleMessung = 5; // Alle Messungen sind abgeschlossen
           if (istTeilfaktoriell) {
             zeigeTeilfaktoriellMessung();
           } else {
             zeigeVollfaktoriellMessung();
           }
         }
         return; // Funktion verlassen
       }
       
       // Eingabe anzeigen
       tft.fillRect(33, 273, 274, 34, TFT_BACKGROUND);
       tft.setTextSize(2);
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(40, 285);
       tft.print(eingabe);
     }
   }
   
   // Eingabe in Float umwandeln
   float mittelwert = eingabe.toFloat();
   
   // Korrekten Mittelwert berechnen zum Vergleich
   float korrekt = berechneMittelwert(messungen, 5);
   
   // Prüfen und moderne Feedback-Anzeige
   bool istKorrekt = abs(mittelwert - korrekt) < 0.1;
   zeigeFeedback(istKorrekt, mittelwert, korrekt, "uW", "Mittelwert");
   
   // Wert speichern (immer den korrekten Wert)
   if (istTeilfaktoriell) {
     teilfaktoriellMittelwerte[versuchIndex] = istKorrekt ? mittelwert : korrekt;
   } else {
     vollfaktoriellMittelwerte[versuchIndex] = istKorrekt ? mittelwert : korrekt;
   }
   
   // Warten auf Nutzer-Bestätigung
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
   }
   
   // Nach erfolgreicher manueller Berechnung: Weiterleitung je nach Kontext
   if (zurueckZurAuswertung) {
     // Zurück zur Auswertung (wenn von Auswertungsbildschirm aufgerufen)
     if (istTeilfaktoriell) {
       zeigeTeilfaktoriellAuswertung();
     } else {
       zeigeVollfaktoriellAuswertung();
     }
   } else {
     // Nächster Versuch oder Auswertung (wenn während Messungen aufgerufen)
     aktuellerVersuch = versuchIndex + 1;
     if (aktuellerVersuch < 8) {
       aktuelleMessung = 0;
       if (istTeilfaktoriell) {
         zeigeTeilfaktoriellMessung();
       } else {
         zeigeVollfaktoriellMessung();
       }
     } else {
       // Alle Versuche abgeschlossen
       if (istTeilfaktoriell) {
         berechneEffekte();
         bestimmeWichtigsteFaktoren();
         manuelleEffektBerechnung();
         zeigeBestaetigung("Alle Messungen abgeschlossen. Zur Auswertung?", TEILFAKTORIELL_AUSWERTUNG);
       } else {
         zeigeBestaetigung("Alle Messungen abgeschlossen. Zur Auswertung?", VOLLFAKTORIELL_AUSWERTUNG);
       }
     }
   }
 }
 
 // Funktion für manuelle Eingabe von Standardabweichungen durch Studenten
 void WindTurbineExperiment::manuelleStandardabweichungEingabe(bool istTeilfaktoriell, int versuchIndex, bool zurueckZurAuswertung) {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Standardabweichungs-Berechnung");
   
   // Versuchsinfo
   tft.fillRoundRect(20, 50, 440, 30, 5, TFT_OUTLINE);
   tft.setTextSize(1);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 60);
   tft.print("Versuch: ");
   tft.print(versuchIndex + 1);
   tft.print(istTeilfaktoriell ? " (Teilfaktoriell)" : " (Vollfaktoriell)");
   
   // Messwertebereich
   tft.fillRoundRect(20, 90, 440, 120, 5, TFT_OUTLINE);
   
   float* messungen;
   float mittelwert;
   if (istTeilfaktoriell) {
     messungen = teilfaktoriellMessungen[versuchIndex];
     mittelwert = teilfaktoriellMittelwerte[versuchIndex];
   } else {
     messungen = vollfaktoriellMessungen[versuchIndex];
     mittelwert = vollfaktoriellMittelwerte[versuchIndex];
   }
   
   // Überschrift
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(30, 100);
  tft.println("Messwerte (uW):");
   
   // Messungen anzeigen
   tft.setTextColor(TFT_TEXT);
   
   // Messwerte als Tabelle
   for (int i = 0; i < 5; i++) {
     int y = 120 + i * 17;
     
     // Zellhintergrund
     if (i % 2 == 0) {
       tft.fillRect(30, y-7, 420, 15, 0x1082);
     }
     
     // Messungsnummer
     tft.setCursor(40, y);
     tft.print("Messung ");
     tft.print(i+1);
     tft.print(": ");
     
     // Messwert
     tft.fillRoundRect(150, y-7, 80, 15, 3, TFT_TITLE_BG);
     tft.setCursor(160, y);
     tft.print(messungen[i], 2);
     tft.print(" uW");
     
     // Abweichung vom Mittelwert
     float abweichung = messungen[i] - mittelwert;
     tft.setCursor(250, y);
     tft.print("Abw.: ");
     
     if (abweichung >= 0) {
       tft.setTextColor(TFT_SUCCESS);
       tft.print("+");
     } else {
       tft.setTextColor(TFT_WARNING);
     }
     
     tft.print(abweichung, 2);
     tft.setTextColor(TFT_TEXT);
   }
   
   // Mittelwert anzeigen - kompakter dargestellt
   tft.fillRoundRect(20, 215, 440, 25, 5, TFT_TITLE_BG);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 222);
   tft.print("Mittelwert: ");
   
   // Mittelwert hervorheben
   tft.fillRoundRect(110, 219, 80, 16, 3, TFT_HIGHLIGHT);
   tft.setCursor(120, 222);
     tft.print(mittelwert, 2);
     tft.print(" uW");
   
   // Aufforderungsbereich
   tft.fillRoundRect(20, 245, 440, 25, 5, TFT_SUCCESS);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 252);
   tft.println("Berechnen Sie die Standardabweichung:");
   
   // Eingabefeld - mit gleichen Dimensionen wie bei Mittelwertberechnung
   tft.fillRoundRect(30, 275, 280, 40, 5, TFT_OUTLINE);
   tft.drawRect(32, 277, 276, 36, TFT_HIGHLIGHT);
   
   // Hilfstext am Eingabefeld
   tft.setCursor(35, 287);
   tft.setTextColor(TFT_LIGHT_TEXT);
   tft.print("Wert eingeben...");
   
   // Hilfetext - mit Löschfunktion, größere Box
   tft.fillRoundRect(320, 275, 140, 50, 5, TFT_SUBTITLE);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(330, 280);
   tft.println("* = Dezimalpunkt");
   tft.setCursor(330, 295);
   tft.println("A = Loeschen");
   tft.setCursor(330, 310);
   tft.println("# = Bestaetigen");
   
   // Variablen für die Eingabe
   String eingabe = "";
   bool fertig = false;
   
   // Eingabeschleife
   while (!fertig) {
     char key = keypad.getKey();
     
     if (key) {
       if (key >= '0' && key <= '9') {
         // Ziffer hinzufügen
         eingabe += key;
       } else if (key == '*' && eingabe.indexOf('.') == -1) {
         // Dezimalpunkt hinzufügen (nur einmal)
         eingabe += '.';
       } else if (key == '#') {
         // Eingabe bestätigen
         fertig = true;
       } else if (key == 'A' && eingabe.length() > 0) {
         // Letztes Zeichen löschen
         eingabe = eingabe.substring(0, eingabe.length() - 1);
       } else if (key == 'D') {
         // Zurück-Taste - Eingabe abbrechen und zurück zum korrekten Ort
         fertig = true;
         if (zurueckZurAuswertung) {
           // Zurück zur Auswertung
           if (istTeilfaktoriell) {
             zeigeTeilfaktoriellAuswertung();
           } else {
             zeigeVollfaktoriellAuswertung();
           }
         } else {
           // Zurück zur Messung des korrekten Versuchs
           aktuellerVersuch = versuchIndex;
           aktuelleMessung = 5; // Alle Messungen sind abgeschlossen
           if (istTeilfaktoriell) {
             zeigeTeilfaktoriellMessung();
           } else {
             zeigeVollfaktoriellMessung();
           }
         }
         return; // Funktion verlassen
       }
       
       // Eingabe anzeigen - mit angepasster Position
       tft.fillRect(33, 278, 274, 34, TFT_BACKGROUND);
       tft.setTextSize(2);
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(40, 290);
       tft.print(eingabe);
     }
   }
   
   // Eingabe in Float umwandeln
   float std = eingabe.toFloat();
   
   // Korrekten Wert berechnen
   float korrekt = berechneStandardabweichung(messungen, 5, mittelwert);
   
   // Prüfen und moderne Feedback-Anzeige
   bool istKorrekt = abs(std - korrekt) < 0.1;
   zeigeFeedback(istKorrekt, std, korrekt, "", "Standardabweichung");
   
   // Wert speichern (immer den korrekten Wert)
   if (istTeilfaktoriell) {
     teilfaktoriellStandardabweichungen[versuchIndex] = istKorrekt ? std : korrekt;
   } else {
     vollfaktoriellStandardabweichungen[versuchIndex] = istKorrekt ? std : korrekt;
   }
   
   // Warten auf Nutzer-Bestätigung
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
   }
   
   // Nach erfolgreicher manueller Berechnung: Weiterleitung je nach Kontext
   if (zurueckZurAuswertung) {
     // Zurück zur Auswertung (wenn von Auswertungsbildschirm aufgerufen)
     if (istTeilfaktoriell) {
       zeigeTeilfaktoriellAuswertung();
     } else {
       zeigeVollfaktoriellAuswertung();
     }
   } else {
     // Nächster Versuch oder Auswertung (wenn während Messungen aufgerufen)
     aktuellerVersuch = versuchIndex + 1;
     if (aktuellerVersuch < 8) {
       aktuelleMessung = 0;
       if (istTeilfaktoriell) {
         zeigeTeilfaktoriellMessung();
       } else {
         zeigeVollfaktoriellMessung();
       }
     } else {
       // Alle Versuche abgeschlossen
       if (istTeilfaktoriell) {
         berechneEffekte();
         bestimmeWichtigsteFaktoren();
         manuelleEffektBerechnung();
         zeigeBestaetigung("Alle Messungen abgeschlossen. Zur Auswertung?", TEILFAKTORIELL_AUSWERTUNG);
       } else {
         zeigeBestaetigung("Alle Messungen abgeschlossen. Zur Auswertung?", VOLLFAKTORIELL_AUSWERTUNG);
       }
     }
   }
 }
 
 // Funktion zur manuellen Berechnung der Effekte durch Studenten
 void WindTurbineExperiment::manuelleEffektBerechnung() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Effekt-Berechnung");
   
   // Faktorauswahl
   tft.fillRoundRect(20, 50, 440, 140, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(30, 60);
   tft.println("Waehlen Sie einen Faktor (1-5):");
   
   // Faktoren als anklickbare Buttons
   for (int i = 0; i < 5; i++) {
     int x = (i % 3) * 140 + 40;
     int y = (i / 3) * 50 + 85;
     
     tft.fillRoundRect(x, y, 120, 40, 5, TFT_TITLE_BG);
     tft.setTextColor(TFT_TEXT);
     tft.setCursor(x+10, y+15);
     tft.print(i+1);
     tft.print(": ");
     tft.print(faktorNamen[i]);
   }
   
   // Anleitung
   zeichneStatusleiste("Druecken Sie eine Taste 1-5, um einen Faktor auszuwaehlen.");
   
   // Eingabe des Faktors
   bool faktorGewaehlt = false;
   int faktorIndex = 0;
   
   // Auf Tasteneingabe warten
   while (!faktorGewaehlt) {
     char key = keypad.getKey();
     
     if (key >= '1' && key <= '5') {
       faktorIndex = key - '1'; // 0-basierter Index
       faktorGewaehlt = true;
     }
   }
   
   // Anzeigen der relevanten Daten für diesen Faktor
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   char titel[50];
   sprintf(titel, "Effekt: %s", faktorNamen[faktorIndex]);
   zeichneTitelbalken(titel);
   
   // Versuche mit niedriger Stufe
   tft.fillRoundRect(20, 50, 210, 120, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextSize(1);
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(30, 60);
   tft.println("Versuche mit niedriger Stufe (-):");
   
   float summeNiedrig = 0;
   int anzahlNiedrig = 0;
   
   // Versuche mit niedriger Stufe anzeigen
   int yNiedrig = 80;
   for (int i = 0; i < 8; i++) {
     if (teilfaktoriellPlan[i][faktorIndex] == -1) {
       // Hintergrund
       if (anzahlNiedrig % 2 == 0) {
         tft.fillRect(30, yNiedrig-7, 190, 15, 0x1082);
       }
       
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(40, yNiedrig);
       tft.print("V");
       tft.print(i + 1);
       tft.print(": ");
       tft.print(teilfaktoriellMittelwerte[i], 2);
       tft.print(" uW");
       
       summeNiedrig += teilfaktoriellMittelwerte[i];
       anzahlNiedrig++;
       yNiedrig += 17;
     }
   }
   
   // Versuche mit hoher Stufe
   tft.fillRoundRect(250, 50, 210, 120, 5, TFT_OUTLINE);
   
   // Überschrift
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.setCursor(260, 60);
   tft.println("Versuche mit hoher Stufe (+):");
   
   float summeHoch = 0;
   int anzahlHoch = 0;
   
   // Versuche mit hoher Stufe anzeigen
   int yHoch = 80;
   for (int i = 0; i < 8; i++) {
     if (teilfaktoriellPlan[i][faktorIndex] == 1) {
       // Hintergrund
       if (anzahlHoch % 2 == 0) {
         tft.fillRect(260, yHoch-7, 190, 15, 0x1082);
       }
       
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(270, yHoch);
       tft.print("V");
       tft.print(i + 1);
       tft.print(": ");
       tft.print(teilfaktoriellMittelwerte[i], 2);
       tft.print(" uW");
       
       summeHoch += teilfaktoriellMittelwerte[i];
       anzahlHoch++;
       yHoch += 17;
     }
   }
   
   // Mittelwertberechnungen - OHNE Anzeige des fertigen Effekts
   tft.fillRoundRect(20, 175, 440, 65, 5, TFT_OUTLINE);
   
   // Mittelwerte anzeigen
   float mittelwertNiedrig = summeNiedrig / anzahlNiedrig;
   float mittelwertHoch = summeHoch / anzahlHoch;
   
   // Mittelwerte in einer Zeile
   tft.setTextColor(TFT_SUBTITLE);
   tft.setCursor(30, 185);
   tft.print("Mittelwert niedrig (");
   tft.setTextColor(TFT_LIGHT_TEXT);
   tft.print("-");
   tft.setTextColor(TFT_SUBTITLE);
   tft.print("): ");
   
   // Wert in Box
   tft.fillRoundRect(200, 182, 70, 15, 3, 0x1082);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(210, 185);
   tft.print(mittelwertNiedrig, 2);
   tft.print(" uW");
   
   // Mittelwert hoch in zweiter Zeile
   tft.setTextColor(TFT_SUBTITLE);
   tft.setCursor(30, 205);
   tft.print("Mittelwert hoch (");
   tft.setTextColor(TFT_HIGHLIGHT);
   tft.print("+");
   tft.setTextColor(TFT_SUBTITLE);
   tft.print("): ");
   
   // Wert in Box
   tft.fillRoundRect(200, 202, 70, 15, 3, 0x04FF);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(210, 205);
   tft.print(mittelwertHoch, 2);
   tft.print(" uW");
   
   // HIER WIRD DER EFFEKT NICHT MEHR ANGEZEIGT - Student soll selbst rechnen
   tft.setTextColor(TFT_SUBTITLE);
   tft.setCursor(30, 225);
   tft.print("Effekt = Mittelwert hoch - Mittelwert niedrig");
   
   // Eingabebereich - höher positioniert
   tft.fillRoundRect(20, 245, 440, 30, 5, TFT_SUCCESS);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 255);
   tft.println("Berechnen Sie den Effekt und geben Sie ihn ein:");
   
   // Eingabefeld - höher positioniert
   tft.fillRoundRect(30, 280, 280, 30, 5, TFT_OUTLINE);
   tft.drawRect(32, 282, 276, 26, TFT_HIGHLIGHT);
   
   // Hilfetext - erweitert mit Löschfunktion, größere Box
   tft.fillRoundRect(320, 270, 140, 50,5, TFT_SUBTITLE);
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(330, 275);
   tft.println("* = Dezimalpunkt");
   tft.setCursor(330, 290);
   tft.println("A = Loeschen");
   tft.setCursor(330, 305);
   tft.println("# = Bestaetigen");
   
   // Variablen für die Eingabe
   String eingabe = "";
   bool fertig = false;
   
   // Eingabeschleife
   while (!fertig) {
     char key = keypad.getKey();
     
     if (key) {
       if (key >= '0' && key <= '9') {
         // Ziffer hinzufügen
         eingabe += key;
       } else if (key == '*' && eingabe.indexOf('.') == -1) {
         // Dezimalpunkt hinzufügen (nur einmal)
         eingabe += '.';
       } else if (key == '#') {
         // Eingabe bestätigen
         fertig = true;
       } else if (key == '0' && eingabe.length() == 0) {
         // Minuszeichen am Anfang
         eingabe += '-';
       } else if (key == 'A' && eingabe.length() > 0) {
         // Letztes Zeichen löschen
         eingabe = eingabe.substring(0, eingabe.length() - 1);
       } else if (key == 'D') {
         // Zurück-Taste - Eingabe abbrechen und zurück zum vorherigen Bildschirm
         fertig = true;
         // Zurück zur Auswertung ohne Speichern
         zeigeTeilfaktoriellAuswertung();
         return; // Funktion verlassen
       }
       
       // Eingabe anzeigen - angepasste Position
       tft.fillRect(33, 283, 274, 24, TFT_BACKGROUND);
       tft.setTextColor(TFT_TEXT);
       tft.setCursor(40, 290);
       tft.print(eingabe);
     }
   }
   
   // Eingabe in Float umwandeln
   float eingegebenerEffekt = eingabe.toFloat();
   
   // Korrekten Effekt berechnen
   float korrekterEffekt = mittelwertHoch - mittelwertNiedrig;
   
   // Prüfen und moderne Feedback-Anzeige
   bool istKorrekt = abs(eingegebenerEffekt - korrekterEffekt) < 0.1;
   zeigeFeedback(istKorrekt, eingegebenerEffekt, korrekterEffekt, "uW", "Effekt");
   
   // Wert in Effekte-Array speichern (immer den korrekten Wert)
   effekte[faktorIndex] = istKorrekt ? eingegebenerEffekt : korrekterEffekt;
   
   // Warten auf Nutzer-Bestätigung
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
   }
   
   // Zurück zur Auswertung
   zeigeTeilfaktoriellAuswertung();
 }
 
 // Diagramm-Ansichten
 void WindTurbineExperiment::zeigeHaupteffekteDiagrammAnsicht() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Main Effects Plot");
   
   // Diagramm anzeigen
   zeigeHaupteffekteDiagramm();
   
   // Informationsbox
   tft.fillRoundRect(20, 250, 440, 50, 5, TFT_OUTLINE);
   tft.setTextColor(TFT_SUBTITLE);
   tft.setTextSize(1);
   tft.setCursor(30, 260);
   tft.println("Interpretation:");
   tft.setTextColor(TFT_TEXT);
   tft.setCursor(30, 275);
   tft.println("Steilere Linien bedeuten staerkere Effekte. Positive Steigung zeigt");
   tft.setCursor(30, 290);
   tft.println("an, dass die hohe Stufe (+) zu hoeherer Leistung fuehrt.");
   
   // Anleitung
   zeichneStatusleiste("Druecken Sie den Drehknopf, um zur Datenansicht zurueckzukehren.");
   
   // Warten auf Knopfdruck
   bool warten = true;
   while (warten) {
     if (digitalRead(ENCODER_BUTTON) == LOW) {
       if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
         buttonPressed = true;
         lastDebounceTime = millis();
         warten = false;
       }
     } else {
       buttonPressed = false;
     }
     
     // Keypad abfragen
     char key = keypad.getKey();
     if (key) {
       if (key == '#' || key == 'D') {
         warten = false;
       }
     }
   }
   
   // Zurück zur Datenansicht
   zeigeTeilfaktoriellAuswertung();
 }
 
 void WindTurbineExperiment::zeigeInteraktionsDiagrammAnsicht() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Interaction Plot");
   
   // Diagramm anzeigen
   zeigeInteraktionsDiagramm();
   
   // Informationsbox
   tft.fillRoundRect(20, 260, 440, 40, 5, TFT_OUTLINE);
   tft.setTextColor(TFT_SUBTITLE);
   tft.setTextSize(1);
   tft.setCursor(30, 270);
   tft.println("Nicht-parallele Linien deuten auf Wechselwirkungen zwischen");
   tft.setCursor(30, 285);
   tft.println("den Faktoren hin. Je staerker der Unterschied, desto staerker die Interaktion.");
   
   // Anleitung
   zeichneStatusleiste("Druecken Sie den Drehknopf, um zur Datenansicht zurueckzukehren.");
   
   // Warten auf Knopfdruck
   bool warten = true;
   while (warten) {
     if (digitalRead(ENCODER_BUTTON) == LOW) {
       if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
         buttonPressed = true;
         lastDebounceTime = millis();
         warten = false;
       }
     } else {
       buttonPressed = false;
     }
     
     // Keypad abfragen
     char key = keypad.getKey();
     if (key) {
       if (key == '#' || key == 'D') {
         warten = false;
       }
     }
   }
   
   // Zurück zur Datenansicht
   zeigeTeilfaktoriellAuswertung();
 }
 
 void WindTurbineExperiment::zeigeTeilfaktoriellDiagrammAnsicht() {
   tft.fillScreen(TFT_BACKGROUND);
   
   // Titelbereich
   zeichneTitelbalken("Teilfaktorieller Versuch: Effekt-Diagramm");
   
   // Diagramm zentral anzeigen - X-Koordinate angepasst für bessere Zentrierung
   zeigeEffekteDiagramm(340, 170);
   
   // Informationsbox
   tft.fillRoundRect(20, 260, 440, 40, 5, TFT_OUTLINE);
   tft.setTextColor(TFT_SUBTITLE);
   tft.setTextSize(1);
   tft.setCursor(30, 270);
  tft.println("Die Balkenhoehe zeigt die Staerke des Effekts. Positive Effekte (gruen)");
  tft.setCursor(30, 285);
  tft.println("bedeuten, dass die hohe Stufe zu besseren Ergebnissen fuehrt.");
  
  // Anleitung
  zeichneStatusleiste("Druecken Sie den Drehknopf, um zur Datenansicht zurueckzukehren.");
  
  // Warten auf Knopfdruck
  bool warten = true;
  while (warten) {
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
        buttonPressed = true;
        lastDebounceTime = millis();
        warten = false;
      }
    } else {
      buttonPressed = false;
    }
    
    // Keypad abfragen
    char key = keypad.getKey();
    if (key) {
      if (key == '#' || key == 'D') {
        warten = false;
      }
    }
  }
  
  // Zurück zur Datenansicht
  zeigeTeilfaktoriellAuswertung();
}

void WindTurbineExperiment::zeigeVollfaktoriellDiagrammAnsicht() {
  tft.fillScreen(TFT_BACKGROUND);
  
  // Titelbereich
  zeichneTitelbalken("Vollfaktorieller Versuch: Ergebnis-Diagramm");
  
  // Diagramm zentral anzeigen
  zeigeVollfaktoriellDiagramm(340, 230);
  
  // Informationsbox
  tft.fillRoundRect(20, 260, 440, 40, 5, TFT_OUTLINE);
  tft.setTextColor(TFT_SUBTITLE);
  tft.setTextSize(1);
  tft.setCursor(30, 270);
  tft.println("Die Balkenhoehe zeigt die gemessene Leistung fuer jeden Versuch.");
  tft.setCursor(30, 285);
  tft.println("Vergleichen Sie die Kombinationen, um optimale Einstellungen zu finden.");
  
  // Anleitung
  zeichneStatusleiste("Druecken Sie den Drehknopf, um zur Datenansicht zurueckzukehren.");
  
  // Warten auf Knopfdruck
  bool warten = true;
  while (warten) {
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
        buttonPressed = true;
        lastDebounceTime = millis();
        warten = false;
      }
    } else {
      buttonPressed = false;
    }
    
    // Keypad abfragen
    char key = keypad.getKey();
    if (key) {
      if (key == '#' || key == 'D') {
        warten = false;
      }
    }
  }
  
  // Zurück zur Datenansicht
  zeigeVollfaktoriellAuswertung();
}

void WindTurbineExperiment::zeigeParetoEffekteDiagrammAnsicht() {
  tft.fillScreen(TFT_BACKGROUND);
  
  // Titelbereich
  zeichneTitelbalken("Pareto-Diagramm der Effekte");
  
  // Erklärung des Diagramms mit verbessertem Layout
  tft.fillRoundRect(20, 50, 440, 25, 5, TFT_OUTLINE);
  tft.setTextSize(1);
  tft.setTextColor(TFT_TEXT);
  tft.setCursor(30, 57);
  tft.println("Faktoren nach absteigender Effektstaerke sortiert");
  
  // Diagramm anzeigen - X-Koordinate angepasst für bessere Zentrierung, Y-Koordinate angepasst
  zeigeParetoEffekteDiagramm(340, 230);
  
  // Faktornamen-Legende in einem schönen Container
  tft.fillRoundRect(20, 260, 440, 80, 5, TFT_OUTLINE);
  
  // Überschrift der Legende
  tft.fillRect(21, 261, 438, 18, TFT_TITLE_BG);
  tft.setTextColor(TFT_HIGHLIGHT);
  tft.setCursor(25, 265);
  tft.print("Faktoren nach Effektstaerke:");
  
  // Sortierte Liste der Faktoren nach Effektstärke
  bool alleNullWerte = true;
  for (int i = 0; i < 5; i++) {
    if (effekte[i] != 0) {
      alleNullWerte = false;
      break;
    }
  }
  
  // Sortierte Indizes der Faktoren
  int sortierteFaktoren[5];
  float sortierteEffekte[5];
  
  // Daten für die Sortierung vorbereiten
  for (int i = 0; i < 5; i++) {
    sortierteFaktoren[i] = i;
    sortierteEffekte[i] = alleNullWerte ? 
                        (i == 0 ? 0.5 : (i == 1 ? 0.3 : (i == 2 ? 0.2 : (i == 3 ? 0.7 : 0.4)))) : 
                        abs(effekte[i]);
  }
  
  // Sortieren (Bubble-Sort) nach absoluten Effekten
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4 - i; j++) {
      if (sortierteEffekte[j] < sortierteEffekte[j + 1]) {
        // Effekte tauschen
        float tempEffekt = sortierteEffekte[j];
        sortierteEffekte[j] = sortierteEffekte[j + 1];
        sortierteEffekte[j + 1] = tempEffekt;
        
        // Indizes tauschen
        int tempIndex = sortierteFaktoren[j];
        sortierteFaktoren[j] = sortierteFaktoren[j + 1];
        sortierteFaktoren[j + 1] = tempIndex;
      }
    }
  }
  
  // Faktornamen in sortierter Reihenfolge in 2 Spalten anzeigen
  tft.setTextColor(TFT_TEXT);
  for (int i = 0; i < 5; i++) {
    int faktorIndex = sortierteFaktoren[i];
    
    // X-Position für 2-Spalten Layout
    int x = (i < 3) ? 30 : 250;
    int y = 285 + ((i < 3) ? i : (i-3)) * 18;
    
    // Hintergrundbalken für bessere Lesbarkeit
    if (i % 2 == 0) {
      int breite = (i < 3) ? 200 : 190;
      tft.fillRect(x-5, y-3, breite, 17, 0x1082);
    }
    
    tft.setCursor(x, y);
    
    // Kürzel und vollständiger Name
    tft.setTextColor(TFT_HIGHLIGHT);
    tft.print(i+1);
    tft.print(". ");
    tft.setTextColor(TFT_TEXT);
    tft.print(faktorNamen[faktorIndex]);
    
    // Effektwert anzeigen
    tft.print(" (");
    if (alleNullWerte) {
      // Testdaten anzeigen
      float testEffekt = (faktorIndex == 0 ? 0.5 : 
                         (faktorIndex == 1 ? -0.3 : 
                         (faktorIndex == 2 ? 0.2 : 
                         (faktorIndex == 3 ? -0.7 : 0.4))));
      
      // Effektwert farblich markieren
      if (testEffekt > 0) {
        tft.setTextColor(TFT_SUCCESS);
      } else {
        tft.setTextColor(TFT_WARNING);
      }
      
      tft.print(testEffekt, 1);
    } else {
      // Effektwert farblich markieren
      if (effekte[faktorIndex] > 0) {
        tft.setTextColor(TFT_SUCCESS);
      } else {
        tft.setTextColor(TFT_WARNING);
      }
      
      tft.print(effekte[faktorIndex], 2);
    }
    tft.setTextColor(TFT_TEXT);
   tft.print(" uW)");
  }
  
  // Anleitung
  zeichneStatusleiste("Druecken Sie den Drehknopf, um zur Auswertung zurueckzukehren.");
  
  // Warten auf Knopfdruck
  bool warten = true;
  while (warten) {
    if (digitalRead(ENCODER_BUTTON) == LOW) {
      if (!buttonPressed && (millis() - lastDebounceTime > debounceDelay)) {
        buttonPressed = true;
        lastDebounceTime = millis();
        warten = false;
      }
    } else {
      buttonPressed = false;
    }
    
    // Keypad abfragen
    char key = keypad.getKey();
    if (key) {
      if (key == '#' || key == 'D') {
        warten = false;
      }
    }
  }
  
  // Zurück zur Auswertung
  zeigeTeilfaktoriellAuswertung();
}
