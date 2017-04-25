![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH und c't, Heise Zeitschriften Verlag

***

### MaXYposi-Schrittmotorsteuerung 

Natürlich wäre als Bahnsteuerung auch ein normaler Arduino-328 mit dem Standard-GRBL geeignet, wenn man sich auf 3 Achsen und eine Fernsteuerung über einen PC beschränkt. Wir haben unser 644-GRBL allerdings auf 4 Achsen erweitert und 32 I/Os hinzugefügt, außerdem ein Handrad- und Joystick-Jog-Anschluss sowie eine Echtzeit-Positionsanzeige mit LDC 16x2 auf einem (optionalen) Bedienfeld. 

Unser Steuerungs-Projekt für 4 Achsen und vielen I/Os auf einer speziell für CNC-Zwecke ausgelegten Platine. Bitte beachten Sie unbedingt 

- den Artikel in **[Make: 2/2017](https://shop.heise.de/katalog/make-2-2017)**
- unsere **[MaXYPosi-Projektseite](http://www.make-magazin.de/maxyposi)**
- die ständig aktualisierte **[FAQ-Seite](https://heise.de/-3676050)** und das zugehörige Forum.

Das mit SMD-Bauteilen vorbestückte Board wird Ende April verfügbar sein. Hinweise zur Bestellung finden Sie dann auf der **[MaXYposi-Hauptseite](http://www.make-magazin.de/maxyposi)**.

### Stückliste

Die bestückte Platine wird mit einem programmierten ATmega644P (Optiboot-Bootloader und GRBL 1.1f2) ausgeliefert. Es sind noch einige bedrahtete Bauteile zu beschaffen und zu bestücken, unter anderem die Steckverbinder und IC-Fassungen:

    Bauteil                                   Anzahl Name
    Jumper-Pfostenleiste 2-pol.                  6   JP300 JP400 JP401 JP1 JP2 JP3
    Jumper-Pfostenleiste 3-pol.                  1   JP5 (1x SL 1x50G 2,54 auf Länge kürzen)            
    PSS 254/2G Wire-To-Board 2-pol. RM2,54       2   PL7 PL9             
    PSS 254/3G Wire-To-Board 3-pol. RM2,54       1   PL4                 
    PSS 254/4G Wire-To-Board 4-pol. RM2,54       7   PL201 PL202 PL203 PL5 PL6 PL204 PL8
    PSS 254/5G Wire-To-Board 5-pol. RM2,54       2   PL16 PL10           
    PSS 254/6G Wire-To-Board 6-pol. RM2,54       2   PL3 PL2             
    PSS 254/8G Wire-To-Board 8-pol. RM2,54       1   PL15                
    10WP                                         5   PL1 PL400 PL401 PL301 PL300
    16WP                                         1   PL501               
    FL-B 64G4 Federleiste A-B DIN41612           2   CON1 CON2 (bei Bedarf)               
    ATmega644P                                   1   U1 (mitgeliefert)                 
    220µ 63V RM5                                 1   C18                 
    D-Sub 25-pol. Female Print                   1   CON5               
    LM317 TO92                                   1   U500                
    SIL 9-8 10K R-Netzw. 8x 10k                  2   RN300 RN301         
    ULN2803                                      2   U402 U403           
    Quarz 16 MHz HC49U                           1   XTAL1               
    IC-Fassung 40pol DIL                         1
    IC-Fassung 18pol DIL                         2

