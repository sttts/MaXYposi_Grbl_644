![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH und c't, Heise Zeitschriften Verlag

***

### MaXYposi-Schrittmotorsteuerung, Programm für Display-ATmega328p

Das Programm existiert derzeit nur für den Compiler AVRco Pascal von e-Lab.de (kostenlos für ATmega8/88 und alle Programme unter 4 kByte). Ein Port auf Arduino ist in Arbeit.

Das hiermit erstellte Hex-File grbl_display.hex lässt sich trotzdem problemlos mit der Arduino-IDE auf einen ATmega328p mit bereits geflashtem Bootloader (z.B. von Watterott oder Adafruit) überspielen.

#### GRBL-644 anpassen

Um das MaXYposi-GRBL mit unserem Kontrollpult einzusetzen, müssen Sie in der 
Datei **config.h** die Zeilen
```
#define USER_PANEL_SMALL 
// #define USER_PANEL_LARGE 
```
ändern in
```
// #define USER_PANEL_SMALL 
#define USER_PANEL_LARGE 
```
und den Sketch **grblUpload_644.ino** neu kompilieren.

#### Weitere Tipps

Unser Leser **[HorstBaerbel](https://github.com/HorstBaerbel)** hat dankenswerterweisedas 
Programmieren des HEX-Files für den Display-Prozessor genauer beschrieben - im 
Heft wurde die Prozedur nur kurz angerissen. Vielen Dank dafür! :+1: 

* Die Firmware mit einem Arduino als ISP auf den ATMEGA328P zu bringen ist nicht ganz einfach. Hier meine Anleitung (für Linux, sollte aber auch für andere OS gehen):

*  Nach dem Tutorial hier den Arduino mit einem ISP-Sketch versehen und mit dem MaxyPulti verbinden. Im Detail bei einem Arduino UNO oder Pro als ISP: (Arduino ISP->MaxyPulti PL3) ISP1->9, ISP2->2, ISP3->7, ISP4->1, ISP5->4 und Arduino Pin 10->5.

* In den Arduino-IDE-Voreinstellungen "Ausführliche Ausgabe während ... Hochladen" aktivieren.

* Als Board "Arduino/Genuino UNO" und als Programmer "Arduino as ISP" einstellen und auf "Bootloader brennen" klicken. Nun sollte der Bootloader installiert sein.

* Die erste Kommandozeile aus dem Ausgabefenster kopieren (fett markierter Teil). Sie sollte in etwa so aussehen: "/home/USERNAME/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino9/bin/avrdude -C/home/USERNAME/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino9/etc/avrdude.conf -v -patmega328p -cstk500v1 -P/dev/ttyACM0 -b19200 -Uflash:w:/home/USERNAME/.arduino15/packages/arduino/hardware/avr/1.6.21/bootloaders/optiboot/optiboot_atmega328.hex:i -Ulock:w:0x0F:m". Hier ist USERNAME der Name des angemeldeten Benutzers.

* Die Datei "grbl_display.hex" herunterladen. Terminal öffnen und in das Verzeichnis mit der Datei wechseln.

* Kommandozeile für HEX-Upload zusammenbauen (oben kopierter Teil, plus fett markierter Teil): "/home/USERNAME/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino9/bin/avrdude -C/home/USERNAME/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino9/etc/avrdude.conf -v -patmega328p -cstk500v1 -P/dev/ttyACM0 -b19200 -Uflash:w:grbl_display.hex -D".

* Nachdem der Upload fertig ist, sollte LED1 auf dem MaxyPulti rhythmisch blinken.
