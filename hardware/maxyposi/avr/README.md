#### MaXYposi Bootloader

Derived from **Sanguino by Kristian Sloth Lauszus, 2012**, MaXYposi additions by -cm


Nur für "jungfräuliche" Controller ohne installierten Bootloader!

Falls noch nicht vorhanden, kann mit einem ISP-Programmer (oder einem dazu umfunktionierten Arduino) dieser Bootloader auf den MaXYposi-Controller ATmega644P geflasht werden. Die Befehlszeilen für avrdude lauten etwa wie folgt (beachten Sie die Fuses-Settings):

avrdude.exe -C "avrdude.conf" -pm644p -cstk500v2 -B8 -P COM%1 -V -s -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0xFC:m -U lock:w:0xEF:m

avrdude.exe -C "avrdude.conf" -pm644p -cstk500v2 -B2 -P COM%1 -U flash:w:"optiboot_atmega644p.hex":i


_________

The code is released under the GNU General Public License.

This content is needed so it's compatible with the Arduino IDE 1.5.x.

For more information see the following sites: [http://www.arduino.cc/en/Guide/Environment#thirdpartyhardware](http://www.arduino.cc/en/Guide/Environment#thirdpartyhardware) and [http://code.google.com/p/arduino/wiki/Platforms1](http://code.google.com/p/arduino/wiki/Platforms1)
or send me an email at <a href="mailto:lauszus@gmail.com?Subject=Sanguino">lauszus@gmail.com</a>.
