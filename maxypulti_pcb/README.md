![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH und c't, Heise Zeitschriften Verlag

***

### MaXYposi-Schrittmotorsteuerung, Stückliste Kontrollpult

In diesem Unterverzeichnis finden Sie die Stücklisten, Schalt- und Bestückungspläne 
zur MaXYposi-Schrittmotorsteuerung, Kontrollpult.


    Bauteil                                   Anzahl Name/Bemerkung
    =============================================================================
    
    PSS 254/4G, Wire-To-Board 4-pol. RM2,54      1   PL6
    PSS 254/5G, Wire-To-Board 4-pol. RM2,54      2   PL1 PL2
    PSS 254/6G, Wire-To-Board 4-pol. RM2,54      2   PL1 PL4    
    Zweireihige Pfostenleiste RM 2.54            1   PL3                 
    WSL 16, Wannenstecker 16pol.                 1   PL5               
    Jumper 2pol. RM2.54                          1   JP1                 
    14WP, Zweireihige Pfostenleiste RM 2.54      1   PL7 (siehe Text unten)                

    74HC165 DIL                                  4   U4 U5 U6 U7         
    74LS595 DIL                                  2   U8 U9               
    ATmega328-DIL                                1   U3  (ggf. mit Bootloader)               
    DS26LS32CM DIL                               1   U1                  
    22p RM5                                      2   C3 C5               
    100n RM5                                     8   C6 C4 C1 C10 C11 C12 C13 C14           
    10µ 63V                                      4   C7 C9  C2 C8              
    LCD alphanum 2x16 Zeichen                    1   U2                  
    LED rot 3mm                                  1   LED1                
    4k7                                          1   R6                  
    10k                                          1   R2                  
    10R                                          1   R3                  
    220R                                         3   R7 R8 R9            
    1K                                          16   R15 R20 R21 R22 R23 R24 R25 R26 R27 R28 R29 R30 R31 R32 R33 R34
    4k7                                          1   R19                 
    22k                                          1   R4                  
    100R                                         8   R12 R10 R11 R13 R14 R16 R17 R18
    SIL 9-8 10k Netzwerk 8-fach, 9-pin           4   RN3 RN4 RN5 RN6     
    Print-Poti Alps, RK11K113-LOG10K             1   P1  (auch RK11K113-LIN10K oder 50K möglich)     
    Trimmer 5mm, 2k5                             1   R1                  
    Taster Multimec 3FTH9                       30   SW1..SW34
    Taster Multimec 3ATH9                        4   SW3, SW6, SW8, SW9   
    Quartz 16 MHz HC49U                          1   XTAL1 
                  
-------------------------------------------------------------------

Beachten Sie die Hinweise im Artikel zu den LED-Vorwiderständen. Diese sind hier 
pauschal mit 1k für blaue/weiße LEDs angegeben, müssen aber für eine ungefähr 
gleiche Helligkeit der einzelnen Farben angepasst werden.

PL7 kann alternativ für ein Display mit seitlichem 14pol. Anschluss verwendet 
werden und wird bei 16x2-Standard-Displays nicht benötigt. Hierfür sollten Sie 
eine einreihige Pfostenleiste RM 2,54 mm mitbestellen.

Die LEDs für die Taster sowie die Tastenkappen sind in der Stückliste NICHT 
aufgeführt. Wählen Sie die Farben nach persönlichem Gusto. Wir haben die Farben 
wie folgt gewählt: Blau = X (Kappe 1D00), Grün = Y (Kappe 1D02), Rot = Z (Kappe 
1D08), Gelb = C-Achse (Kappe 1D04). Die transparent-weißen Tastenkappen haben 
die Bezeichnung 1D16, die schwarzen heißen 1D09. beachten Sie zur Auswahl der 
Farben auch die Bilder im Artikel.

[Flachkopf-LEDs von LED1.de](https://www.led1.de/shop/standard-leds/led-3mm-ultrahell-flat-top/)

[Tastenkappen Serie 1Dxx von MEC Switches, Beispielfarbe Rot, bei Reichelt](https://www.reichelt.de/Kurzhubtaster/KAPPE-1D08/3/index.html?ACTION=3&LA=3&ARTICLE=79710&GROUPID=7587&trstct=vrt_pdn)

[Tastenkappe Serie 1ZCxx von MEC Switches, Jogpad-Mittelkreis, Beispielfarbe Schwarz, bei Reichelt](https://www.reichelt.de/Kurzhubtaster/KAPPE-1ZC09/3/index.html?ACTION=3&LA=2&ARTICLE=79739)

[Tastenkappen Serie 1ZBxx von MEC Switches, Jogpad-Außenkreis, Beispielfarbe Schwarz, bei Reichelt](https://www.reichelt.de/Kurzhubtaster/KAPPE-1ZB09/3/index.html?ACTION=3&LA=517&ARTICLE=79744)

Als Prozessor empfehlen wir einen ATmega328p mit bereits installiertem 
Bootloader, erhältlich zum Beispiel bei [Watterott 
electronic](http://www.watterott.com/de/ATmega328-Arduino-Bootloader); damit ist 
das Flashen der GRBL-Display-Firmware über die Arduino-IDE möglich: Geben Sie als Board den Arduino Nano an und verwenden Sie einen USB-Seriell-Wandler mit FTDI-Chip zur . Ansonsten 
benötigen Sie zum Flashen [einen ISP-Programmer](https://www.reichelt.de/Programmer-Entwicklungstools/DIAMEX-PROG-S2/3/index.html?ACTION=3&LA=446&ARTICLE=164172).

Bitte denken Sie bei einer Bestellung auch an Zubehör wie Flachbandkabel, Steck-
und Pfostenverbinder.

Wer unbestückte Platinen fertigen (lassen) will, kann die Gerber- und 
Bohrdateien "maxypulti_gerber.zip" herunterladen. Bitte beachten Sie: Das Layout 
unterliegt unserem Copyright und darf **nur für private Zwecke** verwendet 
werden. Für eine kommerzielle Nutzung [kontaktieren Sie bitte die 
Redaktion](https://www.heise.de/make/kontakt/).

Vorlagen für die Frontplatte liegen in zwei Versionen vor: Mit und ohne 
Ausschnitt für ein CNC-Handrad (Dial), jeweils als DXF- und als CorelDraw-Datei. 
Die Vorlagen eignen sich zum Plexiglas-Zuschnitt mit Beschriftung bei einem 
Dienstleister Ihrer Wahl, z.B. www.formulor.de oder einem beliebigen Fablab mit 
Lasercutter.

Um das MaXYposi-GRBL mit unserem Kontrollpult einzusetzen, müssen Sie in der 
Datei config.h die Zeilen

  #define USER_PANEL_SMALL<br> 
  // #define USER_PANEL_LARGE 

ändern in

   // #define USER_PANEL_SMALL<br> 
   #define USER_PANEL_LARGE 

und den Sketch grblUpload_644.ino neu kompilieren.


