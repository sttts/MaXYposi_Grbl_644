/***************************************************************************

Dieser Sketch kompiliert GRBL-644 und lädt es auf die MaXYposi-Platine. Bitte 
richten Sie die Arduino-IDE folgendermaßen ein:

- Verschieben Sie das Verzeichnis "grblUpload_644" vom Github-Download in Ihren "Arduino"-Sketch-Ordner.
- Verschieben Sie das Verzeichnis "hardware" vom Github-Download in Ihren "Arduino"-Sketch-Ordner.
- Verschieben Sie das Verzeichnis "libraries\grbl_644" vom Github-Download nach "Arduino\libraries\grbl_644".
- Öffnen Sie den Sketch "..\Arduino\grblUpload_644\grblUpload_644.ino"
- Wählen Sie im Menü "Werkzeuge:Board" den Eintrag "MaXYposi".
- Wählen Sie im Menü "Werkzeuge:Prozessor" den Eintrag "ATmega644A" oder "ATmega644P", je nachdem, welche Variante Sie verwenden.

Sie können nun GRBL mit der Arduino-IDE kompilieren und auch auf das Board 
hochladen; die mit der Platine gelieferten prozessoren sind bereits mit einem 
Sanguino-Optiboot-kompatiblen Bootloader ausgestattet.

****************************************************************************

This sketch compiles and uploads Grbl_644 to your 644p-based MaXYposi Board!

GrblUpload changed by -cm to explicitely use grbl_644 library

To use:
- Copy enclosed "hardware" directory into your Arduino directory.
- Copy entire "grbl_644" directory into "libraries" inside your Arduino directory.
- Select "MaXYiposi" board.
- Check if your CPU is ATmega644A or ATmega644P. Select CPU menu accordingly.
- FTDI device's Serial Port in the Tools drop-down menu.

- Then just click 'Upload'. That's it!

For advanced users:

  If you'd like to see what else Grbl can do, there are some additional
  options for customization and features you can enable or disable. 
  Navigate your file system to where the Arduino IDE has stored the Grbl 
  source code files, open the 'config.h' file in your favorite text 
  editor. Inside are dozens of feature descriptions and #defines. Simply
  comment or uncomment the #defines or alter their assigned values, save
  your changes, and then click 'Upload' here. 

Copyright (c) 2015 Sungeun K. Jeon
Released under the MIT-license. See license.txt for details.
***************************************************************************/

#include <grbl_644.h>

// Do not alter this file!

