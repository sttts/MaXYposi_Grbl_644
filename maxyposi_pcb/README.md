![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH und c't, Heise Zeitschriften Verlag

***

### MaXYposi-Schrittmotorsteuerung, Stückliste Mainboard

In diesem Unterverzeichnis finden Sie die Stücklisten, Fotos, Schalt- und Bestückungspläne 
zur MaXYposi-Schrittmotorsteuerung, Mainboard.

Die mit allen SMD-Bauteilen vorbestückte Platine wird mit einem programmierten 
ATmega644A (Optiboot-Bootloader und GRBL 1.1f2) ausgeliefert. Es sind noch 
einige bedrahtete Bauteile zu beschaffen und zu bestücken, unter anderem die 
Steckverbinder und IC-Fassungen. Wir haben hier die Reichelt-Bestellnummern 
angegeben:

    Bauteil                               Anzahl Name/Bemerkung
    =============================================================================
    Jumper-Pfostenleiste 2-pol.              6   JP300 JP400 JP401 JP1 JP2 JP3
    Jumper-Pfostenleiste 3-pol.              1   JP5 
    Pfostenleisten einreihig                 1   PL5, PL15, PL16 (falls benötigt) 
    (für diese Positionen SL 1x50G 2,54 bestellen und auf Länge kürzen)       
    
    PSS 254/2G, Wire-To-Board 2-pol. RM2,54  2   PL7 PL9             
    PSS 254/3G, Wire-To-Board 3-pol. RM2,54  1   PL4                 
    PSS 254/4G, Wire-To-Board 4-pol. RM2,54  6   PL201 PL202 PL203 PL6 PL204 PL8
    PSS 254/5G, Wire-To-Board 5-pol. RM2,54  2   PL16 PL10           
    PSS 254/6G, Wire-To-Board 6-pol. RM2,54  2   PL3 PL2             
    WSL 10, Wannenstecker                    5   PL1 PL400 PL401 PL301 PL300
    WSL 16, Wannenstecker                    1   PL501               
    FL-B 64G4, Federleiste A-B DIN41612      2   CON1 CON2 (falls benötigt)
    RND 205-00179, Steckv. 4pol.             1   SV Stromversorgung    
    RND 205-00201, Stiftleiste 90°           1   SV Stromversorgung    
    220µ 63V RM5                             1   C18                 
    D-SUB BU 25US, D-Sub 25-pol. Female      1   CON5               
    LM 317 TO92                              1   U500                
    (ATmega644A-PU, DIL)                     1   U1 (mitgeliefert!)                 
    SIL 9-8 10K, R-Netzw. 8x 10k             2   RN300 RN301         
    ULN 2803A (DIL)                          2   U402 U403           
    16,0000-HC49U-S, Quarz 16 MHz HC49U      1   XTAL1               
    GS 40P, IC-Fassung 40pol DIL             1
    GS 18P, IC-Fassung 18pol DIL             2

Bitte denken Sie bei einer Bestellung auch an Zubehör wie Flachbandkabel, Steck-
und Pfostenverbinder.

Wer unbestückte Platinen fertigen (lassen) will, kann die Gerber- und 
Bohrdateien "grbl_maxyposi_gerber.zip" herunterladen. Bitte beachten Sie: Das 
Layout unterliegt unserem Copyright und darf **nur für private Zwecke** verwendet 
werden. Für eine kommerzielle Nutzung [kontaktieren Sie bitte die 
Redaktion](https://www.heise.de/make/kontakt/).
