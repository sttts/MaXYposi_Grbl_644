echo off
echo Prepare fuses and flash bootloader
echo with DIAMEX programmer for out-of-the-box 644p
if "%1"=="" ( 
echo ERROR: COM port not specified. 
echo Usage: 
echo diamex_644_fuses XX with XX = COM port # of DIAMEX ISP programmer
echo assigned by Windows Device Manager
goto end
)
avrdude.exe -C "avrdude.conf" -pm644p -cstk500v2 -B8 -P COM%1 -V -s -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0xFC:m -U lock:w:0xEF:m
avrdude.exe -C "avrdude.conf" -pm644p -cstk500v2 -B2 -P COM%1 -U flash:w:"optiboot_atmega644p.hex":i
:end