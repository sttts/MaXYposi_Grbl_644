echo off
echo Flash GRBL for MaXYposi board with DIAMEX programmer
if "%1"=="" ( 
echo ERROR: COM port not specified. 
echo Usage: 
echo diamex644 XX with XX = COM port # of DIAMEX ISP programmer
echo assigned by Windows Device Manager
goto end
)
avrdude.exe -C "avrdude.conf" -pm644p -cstk500v2 -B2 -P COM%1 -V -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0xFC:m -U lock:w:0xEF:m -U flash:w:"grblUpload_644.ino.with_bootloader.maxyposi.hex":i
:end