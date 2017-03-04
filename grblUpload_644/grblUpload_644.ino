/***************************************************************************
This sketch compiles and uploads Grbl_644 to your 644p-based MaXYposi Board!
Install "Sanguino" booard addition from  
https://raw.githubusercontent.com/Lauszus/Sanguino/master/package_lauszus_sanguino_index.json

GrblUpload changed by -cm to explicitely use grbl_644 library

To use:
- Copy enclosed "hardware" directory into your Arduino directory.
- Copy entire "grbl_644" directory into "libraries" inside your Arduino directory.
- Select "MaXYiposi" board and FTDI device's Serial Port in the Tools drop-down menu.

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

