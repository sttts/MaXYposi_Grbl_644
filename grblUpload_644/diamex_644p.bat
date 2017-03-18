echo off
echo Flash GRBL for MaXYposi board with DIAMEX programmer
if "%1"=="" ( 
echo ERROR: COM port not specified. 
echo Usage: 
echo diamex644 XX with XX = COM port # of DIAMEX ISP programmer
echo assigned by Windows Device Manager
goto end
)
avrdude.exe -C "avrdude.conf" -pm644p -cstk500v2 -B2 -P COM%1 -V -U flash:w:"grblUpload_644.ino.maxyposi.hex":i
:end