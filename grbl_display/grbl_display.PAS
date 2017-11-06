program GRBL_DISPLAY;
// SPI-Empänger für GRBL auf ATmega328 oder 644 mit cm-Firmware 0.9ks oder 1.1f2
// Beispielantwort:
// M,0.58,5.05,-0.96,W,0.58,5.05,-0.96,S,0,B,0
// oder
// NO RESP
// wenn kein SPI empfangen

// Kodierung S = STATE bei GRBL:
// STATE_IDLE          0      // Must be zero. No flags.
// STATE_ALARM         bit(0) // In alarm state. Locks out all g-code processes. Allows settings access.
// STATE_CHECK_MODE    bit(1) // G-code check mode. Locks out planner and motion only.
// STATE_HOMING        bit(2) // Performing homing cycle
// STATE_CYCLE         bit(3) // Cycle is running or motions are being executed.
// STATE_HOLD          bit(4) // Active feed hold
// STATE_SAFETY_DOOR   bit(5) // Safety door is ajar. Feed holds and de-energizes system.
// STATE_MOTION_CANCEL bit(6) // Motion cancel by feed hold and return to idle.
// B = Buttons:
// X REVERSE           bit(0)
// X FORWARD           bit(1)
// Y REVERSE           bit(2)
// Y FORWARD           bit(3)
// Z REVERSE           bit(4)
// Z FORWARD           bit(5)
// ZERO ALL            bit(6)
// SPINDLE_ON          bit(7)
{$NOSHADOW}
{ $WG}                     {global Warnings off}

Device = mega168, VCC=5;
{ $BOOTRST $07000}         {Reset Jump to $07000}

Define_Fuses
Override_Fuses;    // optional, always replaces fuses in ISPE
COMport = USB;     // COM2..COM7, USB
ProgMode = SPI;    // SPI, JTAG or OWD
ProgFuses = true;
ProgFlash = true;

Import SysTick, SerPort, LCDPort;

From System Import Float;

Define
  ProcClock      = 16000000;       {Hertz}
  StackSize      = $0064, iData;
  FrameSize      = $0064, iData;
  SysTick        = 10;              //msec
  SerPort        = 19200, Stop1;    {Baud, StopBits|Parity}
  RxBuffer       = 32, iData;
  TxBuffer       = 128, iData;
  LCDport        = PortD, 5, PortC, 0;   // controlport, bit, Dataport, bit
  LCDtype        = 44780; // 44780 oder 66712;
  LCDrows        = 2;     // rows
  LCDcolumns     = 16;     // chars per line

Implementation

var
{$DATA}
  irq_incr0, irq_incr1, irq_incr2,
  irq_data, irq_data_old1, irq_data_old2, irq_data_old3: byte;
  irq_rx_idx: byte;

{$PDATA}
   SS[@PinB,2] : bit;
   LED[@PortD,2] : bit;
   
{$IDATA}

  i : byte;
  irq_got_header, irq_data_ready, irq_sema: Boolean;
  irq_axis_ptr, b: byte;
  irq_rx_table: array[0..63] of byte;
  mpos_x[@irq_rx_table]: Float;
  wpos_x[@irq_rx_table+ 4]: Float;
  mpos_x_bin[@irq_rx_table]: LongInt;
  wpos_x_bin[@irq_rx_table+ 4]: LongInt;

  mpos_y[@irq_rx_table+ 16]: Float;
  wpos_y[@irq_rx_table+ 20]: Float;
  mpos_y_bin[@irq_rx_table+ 16]: LongInt;
  wpos_y_bin[@irq_rx_table+ 20]: LongInt;

  mpos_z[@irq_rx_table+ 32]: Float;
  wpos_z[@irq_rx_table+ 36]: Float;
  mpos_z_bin[@irq_rx_table+ 32]: LongInt;
  wpos_z_bin[@irq_rx_table+ 36]: LongInt;

  mpos_c[@irq_rx_table+ 48]: Float;
  wpos_c[@irq_rx_table+ 52]: Float;
  mpos_c_bin[@irq_rx_table+ 48]: LongInt;
  wpos_c_bin[@irq_rx_table+ 52]: LongInt;

  mpos_x_bin_old: LongInt;
  wpos_x_bin_old: LongInt;

  mpos_y_bin_old: LongInt;
  wpos_y_bin_old: LongInt;

  mpos_z_bin_old: LongInt;
  wpos_z_bin_old: LongInt;
  
  mpos_c_bin_old: LongInt;
  wpos_c_bin_old: LongInt;

  send_data: byte;
  sys_state, sys_state_old: byte;
  button_number, button_number_old: byte;

  wpox_x_str: String[15];
  wpox_y_str: String[15];
  wpox_z_str: String[15];
  wpox_c_str: String[15];
  mpox_x_str: String[15];
  mpox_y_str: String[15];
  mpox_z_str: String[15];
  mpox_c_str: String[15];
  changed_x, changed_y, changed_z, changed_s,
  changed_c, keep_c_on_display,
  flash_toggle, is_moving, spindle_state: Boolean;
  
  changed, LCDpresent: Boolean;
  led_on_timer, response_timeout, flash_timer, c_displ_timer: Systimer8;
{
  Encoder_short: ShortInt;
  DisplayButtons: Byte;   // invertiert - low=on! Buttons auf Display-Panel
  DisplayButtonDown[@DisplayButtons, 5]  : bit;
  DisplayButtonUp[@DisplayButtons, 4]  : bit;
  DisplayButtonEnter[@DisplayButtons, 3]  : bit;
}
{
procedure onSysTick;
// Selbstgemachter Inkrementalgeber
begin
  irq_incr0:= (PinC and 3);
  if irq_incr0 <> irq_incr1 then
    if irq_incr0 = 0 then // Rastpunkt überschritten
      if (irq_incr2 = 3)then       // kommt aus Ruheposition
        if(irq_incr1 = 1) then     // Rechtsdrehung
          inc(Encoder_short);
        elsif(irq_incr1 = 2) then  // Linksdrehung
          dec(Encoder_short);
        endif;
      endif;
    endif;
    irq_incr2:= irq_incr1;  // vorvorheriger Wert
    irq_incr1:= irq_incr0;  // vorheriger Wert
  endif;
end;


function DisplayButtonPressed:Boolean;
var my_count: word;
begin
  DisplayButtons := not (LCDportInp or %11000111);
  mDelay(20);
  if DisplayButtons = 0 then
    return(false);
  else
    return(true);
  endif;
end;
}

Interrupt SPIRDY;
// SPI-Slave wartet auf Header $FF $AA $55 $0X mit  $0X = Achse 0..2
// und legt dann empfangene Werte in Array ab.
begin
  irq_data:= SPDR;
  irq_sema:= true;
  // Header $FF $AA $55 $0X mit  $0X = Achse 0..3
  if irq_got_header then
    if irq_rx_idx = 8 then
      sys_state:= irq_data;
    elsif irq_rx_idx = 9 then  // letztes Byte
      button_number:= irq_data;
      irq_data_ready:= true;
      irq_got_header:= false;
      irq_rx_idx:= 0;
      SPDR:= send_data; // Send SPI Command
    else
      irq_rx_table[irq_axis_ptr + irq_rx_idx]:= irq_data;
    endif;
    inc(irq_rx_idx);
  endif;
  
  if (irq_data < 4 )         // Axis index
   and (irq_data_old1 = 85)  // Funny complement Header $FF $AA $55
   and (irq_data_old2 = 170)
   and (irq_data_old3 = 255) then
    irq_got_header:= true;
    irq_data_ready:= false;
    irq_rx_idx:= 0;
    irq_axis_ptr:= irq_data shl 4;
  endif;

  irq_data_old3:= irq_data_old2;
  irq_data_old2:= irq_data_old1;
  irq_data_old1:= irq_data;
end;

procedure InitPorts;
begin
  DDRB:=  %00000000;
  PortB:= %11111111;
  
  DDRC:=  %00000000;
  PortC:= %11111111;
  
  DDRD:=  %11100110;
  PortD:= %00011111;
  
  SPCR := %11000000;          // Enable SPI, Slave, CPOL/CPHA=1,1 Mode 3
  SPSR := %00000000;          // %00000001 = Double Rate, %00000000 = Normal Rate
  
  LED:= true;
  LCDCharSet(#0, $1F, $1F, $1F, $1F, $1F, $1F, $1F, $00); // full bar
  LCDCharSet(#1, $1F, $18, $18, $18, $18, $18, $1F, $00); // half bar
  LCDCharSet(#2, $10, $10, $10, $10, $10, $10, $10, $00); // end bar
  LCDCharSet(#3, $00, $04, $04, $0E, $0E, $1F, $1f, $00); // Up Arrow
  LCDCharSet(#4, $1F, $1F, $0E, $0E, $04, $04, $00, $00); // Down Arrow
  LCDCharSet(#5, $10, $18, $1C, $1E, $1C, $18, $10, $00); // ">" Cursor
  LCDCharSet(#6, $01, $03, $07, $0F, $07, $03, $01, $00); {"<" Cursor}
//    LCDCharSet(#7, $04, $0E, $1F, $00, $1F, $0E, $04, $00); {UpDown}
  LCDCharSet(#7, $1F, $00, $00, $00, $00, $00, $1F, $00); // empty bar
  LCDcursor(false, false);
  LCDpresent:= true;

  LCDxy(0, 0);
  write(LCDOut, 'GRBL DISPLAY');
  LCDxy(0, 1);
  write(LCDOut, 'by CM 7/2016');
  EnableInts;
  mdelay(1000);
  LCDclr;
  LED:= false;
end InitPorts;

procedure UpdateDisplay;
begin
  if LCDpresent then
    if changed_x then
      LCDxy(0, 0);
      LCDout('X');
      write(LCDout, wpox_x_str);
      LCDout(#32);
    endif;
    if changed_y then
      LCDxy(0, 1);
      LCDout('Y');
      write(LCDout, wpox_y_str);
      LCDout(#32);
    endif;
    if changed_c then
      LCDxy(9, 0);
      LCDout('C');
      write(LCDout, wpox_c_str);
    endif;
    if changed_z then
      LCDxy(9, 0);
      LCDout('Z');
      write(LCDout, wpox_z_str);
    endif;
    LCDxy(15, 1);
    if (sys_state >= 128) then
      LCDout('*');
    else
      LCDout(#32);
    endif;
    LCDxy(9, 1);
    case (sys_state and $7F) of
      0:
        write(LCDout, 'Idle  ');
      |
      1:
        if flash_toggle then
          write(LCDout, 'Alarm!');
        else
          write(LCDout, '      ');
        endif;
      |
      2:
        write(LCDout, 'Check ');
      |
      4:
        write(LCDout, 'HomeC ');
      |
      8:
        write(LCDout, 'Run   ');
      |
      16:
        write(LCDout, 'Hold  ');
      |
      32:
        write(LCDout, 'Jog   ');
      |
    else
      if flash_toggle then
        write(LCDout, 'NoResp!');
      else
        write(LCDout, '       ');
      endif;
    endcase;
  endif;
end;

procedure ShowButtonOnOff(button_number: byte);
begin
  if (button_number and 1) = 1 then
    write(LCDout, ' ON ');
  else
    write(LCDout, ' OFF');
  endif;
end;

procedure ShowButtonMessage;
begin
  if (button_number > 0) and (button_number < 100) then
    LCDxy(0,1);
    LCDclreol;
    case button_number of
      1:// Homing
        write(LCDout, 'Home Cycle Run');
        mdelay(1000);
        |
      2:// Clear Alarm
        write(LCDout, 'Alarm Cleared');
        mdelay(1000);
        |
      3:// ZeroX
        write(LCDout, 'X Zero Set');
        |
      4:// ZeroY
        write(LCDout, 'Y Zero Set');
        |
      5:// ZeroZ
        write(LCDout, 'Z Zero Set');
        |
      6:// ZeroC
        write(LCDout, 'C Zero Set');
        |
      7:// ZeroAll
        write(LCDout, 'All Zero Set');
        |
      8:// Feed Inc
        write(LCDout, 'Feed +10%');
        |
      9:// Feed Dec
        write(LCDout, 'Feed -10%');
        |
      10, 11:// Flood
        write(LCDout, 'Flood Cool');
        ShowButtonOnOff(button_number);
        |
      12,13:// Mist
        write(LCDout, 'Mist Cool');
        ShowButtonOnOff(button_number);
        |
      14,15:// ATC
        write(LCDout, 'ATC Clamp');
        ShowButtonOnOff(button_number);
        |
      16,17:// AUX1
        write(LCDout, 'AUX1 Relay');
        ShowButtonOnOff(button_number);
        |
      18,19:// AUX2
        write(LCDout, 'AUX2 Relay');
        ShowButtonOnOff(button_number);
        |
      20,21:// AUX3
        write(LCDout, 'AUX3 Relay');
        ShowButtonOnOff(button_number);
        |
      22:// Spindle inc
        write(LCDout, 'Spindle +10%');
        |
      23:// Spindle dec
        write(LCDout, 'Spindle -10%');
        |
      24:
        write(LCDout, 'XY Zero Set');
        |
      25:
        write(LCDout, 'Dial X Select');
        |
      26:
        write(LCDout, 'Dial Y Select');
        |
      27:
        write(LCDout, 'Dial Z Select');
        |
      28:
        write(LCDout, 'Dial C Select');
        |
      30,31:
        write(LCDout, 'Dial Fast');
        ShowButtonOnOff(button_number);
        |
      32,33:
        write(LCDout, 'Spindle ');
        ShowButtonOnOff(button_number);
        |
      34:// ZeroXY
        write(LCDout, 'X,Y Zero Set');
        |
      35..98:
        return;
        |
      99:
        write(LCDout, 'Func Disabled');
        |
    else
      return;
    endcase;
    mdelay(750);
    wpos_x_bin_old:= 9999;
    wpos_y_bin_old:= 9999;
    wpos_z_bin_old:= 9999;
    sys_state_old:= 127;
  endif;
end;


function FlashToggleEvent: Boolean;
begin
  if isSysTimerZero(flash_timer) then
    flash_toggle:= not flash_toggle;
    setSystimer(flash_timer, 50);
    changed_s:= true;
    return(true);
  else
    return(false);
  endif;
end;

{--------------------------------------------------------------}
{ Main Program }
{$IDATA}

begin
  sys_state_old:= 127;
  InitPorts;
  SetSystimer(response_timeout, 25);
  wpos_x_bin_old:= 9999;
  wpos_y_bin_old:= 9999;
  wpos_z_bin_old:= 9999;
  sys_state_old:= 127;
  loop
    repeat
      if serstat then
        b:= serinp; // Button Number from Maximite TFT
      endif;
      if isSysTimerZero(led_on_timer) then
        LED:= false;
      endif;
      if FlashToggleEvent then
        UpdateDisplay;
      endif;
    until (irq_sema and irq_data_ready) or isSysTimerZero(response_timeout);
    if isSysTimerZero(response_timeout) then
      changed_x:= true;
      changed_y:= true;
      changed_c:= keep_c_on_display;
      changed_z:= not keep_c_on_display;   // wieder auf Z
      is_moving:= false;
      changed_s:= true;
      sys_state:= 127; // invalidate
    else
      changed_x:= (wpos_x_bin <> wpos_x_bin_old);
      changed_y:= (wpos_y_bin <> wpos_y_bin_old);
      changed_z:= (wpos_z_bin <> wpos_z_bin_old);
      if not changed_z then
        changed_c:= (wpos_c_bin <> wpos_c_bin_old);
        if changed_c then
          keep_c_on_display:= true;
          SetSystimer(c_displ_timer, 100);
        endif;
      endif;
      if FlashToggleEvent or (sys_state <> sys_state_old) then
        changed_s:= true;
        changed_x:= true;
        changed_y:= true;
        changed_z:= true;
      endif;
      is_moving:= changed_x or changed_y or changed_z or changed_c;
    endif;

    SetSystimer(response_timeout, 100);

    if isSysTimerZero(c_displ_timer) then
      if keep_c_on_display then  // war C angezeigt?
        changed_z:= true;   // wieder auf Z, Anzeige erzwingen
      endif;
      keep_c_on_display:= false;
    endif;
    SetSystimer(led_on_timer, 5);
    LED:= not LED;
    irq_data_ready:= false;
    
    if changed_x then
      if abs(wpos_x) < 0.01 then
        wpos_x:= 0;
      elsif wpos_x > 999.9 then
        wpos_x:= 999.9;
      elsif wpos_x < -999.9 then
        wpos_x:= -999.9;
      endif;
      wpox_x_str:= FloatToStr(wpos_x:7:2);
      wpox_x_str[0]:= char(7);
      write(serout, 'XM,');
      write(serout, FloatToStr(mpos_x:3:2));
      write(serout, ',XW,');
      write(serout, wpox_x_str);
      serout(#13);
      UpdateDisplay;
      wpos_x_bin_old:= wpos_x_bin;
      changed_x:= false;
    endif;
    if changed_y then
      if abs(wpos_y) < 0.01 then
        wpos_y:= 0;
      elsif wpos_y > 999.9 then
        wpos_y:= 999.9;
      elsif wpos_y < -999.9 then
        wpos_y:= -999.9;
      endif;
      wpox_y_str:= FloatToStr(wpos_y:7:2);
      wpox_y_str[0]:= char(7);
      write(serout, 'YM,');
      write(serout, FloatToStr(mpos_y:3:2));
      write(serout, ',YW,');
      write(serout, wpox_y_str);
      serout(#13);
      UpdateDisplay;
      wpos_y_bin_old:= wpos_y_bin;
      changed_y:= false;
    endif;
    if changed_z then
      if abs(wpos_z) < 0.01 then
        wpos_z:= 0;
      elsif wpos_z > 99.9 then
        wpos_z:= 99.9;
      elsif wpos_z < -99.9 then
        wpos_z:= -99.9;
      endif;
      wpox_z_str:= FloatToStr(wpos_z:6:2);
      wpox_z_str[0]:= char(6);
      write(serout, 'ZM,');
      write(serout, FloatToStr(mpos_z:3:2));
      write(serout, ',ZW,');
      write(serout, wpox_z_str);
      serout(#13);
      UpdateDisplay;
      wpos_z_bin_old:= wpos_z_bin;
      changed_z:= false;
    endif;
    if changed_c then
      if abs(wpos_c) < 0.01 then
        wpos_c:= 0;
      elsif wpos_c > 999.9 then
        wpos_c:= 999.9;
      elsif wpos_c < -999.9 then
        wpos_c:= -999.9;
      endif;
      wpox_c_str:= FloatToStr(wpos_c:6:1);
      wpox_c_str[0]:= char(6);
      write(serout, 'ZM,');
      write(serout, FloatToStr(mpos_c:3:2));
      write(serout, ',ZW,');
      write(serout, wpox_c_str);
      serout(#13);
      UpdateDisplay;
      wpos_c_bin_old:= wpos_c_bin;
      changed_c:= false;
    endif;
    if changed_s then
      write(serout, 'S,');
      write(serout, ByteToStr(sys_state));
      write(serout, ',B,');
      write(serout, ByteToStr(button_number));
      serout(#13);
      UpdateDisplay;
      sys_state_old:= sys_state;
    endif;

    if button_number <> 0 then
      ShowButtonMessage;
    endif;
  endloop;
end.

