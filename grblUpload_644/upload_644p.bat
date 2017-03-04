echo off
echo Flash GRBL to MaXYposi 644P (bootloader installed!) with avrdude
if "%1"=="" ( 
echo ERROR: COM port not specified. 
echo Usage: 
echo upload_644p XX
echo or without verify:
echo upload_644p XX -V
echo with XX = Serial Device COM port number, 
echo assigned by Windows Device Manager
goto end
)
if "%2"=="-V" ( 
echo WARNING: avrdude verify disabled by request.
echo on
avrdude -C "avrdude.conf" -pm644p -carduino -P COM%1 -b115200 -U -V flash:w:"grblUpload_644.ino.maxyposi.hex":i
) else ( 
echo on
echo avrdude verify enabled.
avrdude -C "avrdude.conf" -pm644p -carduino -P COM%1 -b115200 -U flash:w:"grblUpload_644.ino.maxyposi.hex":i
)
:end