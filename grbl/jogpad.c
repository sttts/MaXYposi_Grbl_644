/*
  jogging.c - code pertaining to  jog switches
  Copyright (c) 2013 Carsten Meyer, cm@ct.de

	Needs some changes/hooks in 
	protocol.c, --> protocol_main_loop() 
	stepper.c 	--> ISRs, st_wake_up(), st_go_idle()
	
	Alles noch ziemlich Kraut und Rüben
	by cm@heise.de
 
*/


#include "grbl_644.h"


uint8_t adc_raw, adc_dest_feed, adc_current_feed; // ADC value
float adc_squared;
uint8_t old_f_override;


static uint8_t buttons_old = 0;

volatile int8_t last_pin, last_pin0, last_pin1;		// benutzt in ISR(DIAL_INT_vect)
volatile bool dial_sema;
volatile int8_t dial_delta_8;      // Schritte von Handrad aus ISR(DIAL_INT_vect), Signed Byte!

float dial_delta_f;       // Akkumulator für Handrad-Werte
int32_t dial_delta_32;	           // Akkumulator für Handrad-Werte


void check_encoder_hook(uint8_t dial_pin) {
// Pin change interrupt hook for manual dial, bits DIAL_PHA, DIAL_PHB
// für "atomare" Operation im Limit-Pinchange-Interrupt ist dial_delta_8 ein Signed Byte! 
	if (sys.state & (STATE_ALARM | STATE_HOLD)) return;
		
  uint8_t pin = (dial_pin & DIAL_MASK) >> DIAL_0_BIT;

  if (last_pin == pin) return; // no change, sporadic interrupt
  last_pin = pin;
  
	// Forward: Folge 013201320...
  if ((last_pin1 == 3) & (last_pin0 == 2) & (pin == 0)) { 
    dial_delta_8++; 
  	dial_sema = true;
    }
	// Reverse: Folge 023102310...
  if ((last_pin1 == 3) & (last_pin0 == 1) & (pin == 0)) { 
    dial_delta_8--; 
  	dial_sema = true;
    }
  last_pin1 = last_pin0;
  last_pin0 = pin;
  if (dial_sema) {
		#ifdef debug_dial
			// Show state sequence of wheel:
	    // serial_write(pin + 48);
		  serial_write('*');
		  print_uint8_base10(dial_delta_8);
		  printPgmString(PSTR("\r\n"));
		  serial_write('*');
		#endif
	}
}

//#############################################################################################
//#############################################################################################

/*
// ist jetzt SR-/OE
void set_activity_led() {
	ACTIVITY_PORT &= ~(1 << ACTIVITY_LED);
	activity_count = 2000;	
}
*/

void jogpad_init() {

#ifdef DIAL
  dial_delta_32 = 0;
	dial_delta_8 = 0;						// damit nichts überläuft
	dial_sema = false;
	dial_axis = 0; // default X
	
	LIMIT_DDR &= ~((DIAL_MASK | (1 << LIMIT_SPINDLE_TOGGLE_BIT))); // Set as input pins
  LIMIT_PORT |= DIAL_MASK | (1 << LIMIT_SPINDLE_TOGGLE_BIT); // Active low operation. Pull-up on.
  PCICR |= (1 << LIMIT_INT); // Enable Pin Change Interrupt
  LIMIT_PCMSK |= DIAL_MASK; // Enable specific pins of the Pin Change Interrupt
#endif
#ifdef ANALOG_JOYSTICK
  ADMUX = ((1<<ADLAR) | (1<<REFS0) | JOG_POT);     // Kanal, ADLAR =1 (left adjustet, 8-Bit-Result on ADCH)
  ADCSRA = ((1<<ADEN) | (1<<ADSC) | (1<<ADATE) | (1<< ADPS2) | (1<< ADPS0)) ; // Free running, auto trigger, prescaler 32
  ADCSRB = 0; 
#endif
	activity_count = 0;
  blink_count = 0;
	blink_toggle = 0;
	last_pin = 0;
	last_pin0 = 0;
	last_pin1 = 0;
}

void btn_wait_execute() {
  spi_txrx_inout();
  delay_ms(3);
	spi_tx_axis(0);
  protocol_execute_realtime(); 
  delay_ms(3);
	spi_tx_axis(1);
  protocol_execute_realtime(); 
  delay_ms(3);
	spi_tx_axis(2);
  protocol_execute_realtime(); 
}

void machine_btn_release() {
  while (MACHINE_INP_SR) { 					// until released
    btn_wait_execute(); 
  }
} 
   
void accessory_btn_release() {
  while (ACCESSORY_INP_SR) { 					// until released
    btn_wait_execute(); 
  }
}    

void joy_btn_release() {
  while (JOY_INP_SR & (1<<JOY_ZERO_SW)) { 			// until released
    btn_wait_execute(); 
  }
} 

void dial_btn_release() {
  while (DIAL_INP_SR & (1<<DIAL_ZERO_SW)) { 		// until released
    btn_wait_execute(); 
  }
} 

void jogpad_zero_all() {
	uint8_t idx;
	jog_zero_request_flag = 7;
  for (idx=0; idx<N_AXIS; idx++) {
    gc_state.coord_offset[idx] = gc_state.position[idx];
	}
  system_flag_wco_change(); // Set to refresh immediately, something altered.
}

void set_led_status() {
	uint8_t status_temp = 0;
	uint8_t leds_temp;
/*
	if (activity_count > 0) { 
		activity_count--; 
	} else {
	  ACTIVITY_PORT |= (1 << ACTIVITY_LED);
	}
*/	
	blink_count++;
	// diese Status-LEDs blinken, wenn aktiv
	// LED_ALARM, LED_HOLD, LED_HOMING (muss in limits_go_home()-
	// und protocol_exec_rt_suspend()-Schleife aufgerufen werden)
	if (sys.state & STATE_ALARM) {
		#ifdef LED_ALARM
			leds_temp = (1 << LED_ALARM);
		#else
			leds_temp = 0;
		#endif;
		// aufgeregt blinken bei Alarm
		if (blink_count > 2250) { 
			blink_toggle ^= 0xFF;
			blink_count = 0;
		}
	} else {
		leds_temp = 0;
		if (blink_count > 5000) { 
			blink_toggle ^= 0xFF;
			blink_count = 0;
		}
	}
  #ifdef LED_HOLD
	if (sys.state & STATE_HOLD) {
		leds_temp |= (1 << LED_HOLD);
	}
	#endif;
  #ifdef LED_HOMING
	if (sys.state & STATE_HOMING) {
		leds_temp |= (1 << LED_HOMING);
	}
	#endif;
	leds_temp &= blink_toggle;
	
  #ifdef LED_RUN
	if (sys.state & STATE_CYCLE) {
		leds_temp |= (1 << LED_RUN);
	}
	#endif;
  #ifdef LED_RUN
	if (sys.state & STATE_JOG) {
		leds_temp |= (1 << LED_JOG);
	}
	#endif;
	
  #ifdef LED_SPINDLE
	if (spindle_get_state()) { 
		leds_temp |= (1<<LED_SPINDLE); 
		sr_dispstate_1 = sys.state | 0x80;
	} else {
		sr_dispstate_1 = sys.state;
	}
	#endif;
  
  status_temp = coolant_get_state();
  #ifdef LED_FLOOD
	if (status_temp & COOLANT_STATE_FLOOD) {
		leds_temp |= (1 << LED_FLOOD);
	}
	#endif;
	#ifdef LED_MIST
	if (status_temp & COOLANT_STATE_MIST) {
		leds_temp |= (1 << LED_MIST);
	}
	#endif;
	#ifdef DIAL	
		if (dial_axis == 0) {
			leds_temp |= (1 << LED_DIAL_SELECT_X);
		} else if (dial_axis == 1) {
			leds_temp |= (1 << LED_DIAL_SELECT_Y);
		} else if (dial_axis == 2) {
			leds_temp |= (1 << LED_DIAL_SELECT_Z);
		}
	#endif;
 	STATUS_LED_OUT_SR = leds_temp; 		// Status-LED-Port setzen
 	#ifdef SPI_DISP	
 	sr_dispstate_0 = MACHINE_OUT_SR; 	// wurde in spindle/coolant_control gesetzt
	#endif;
}


//#############################################################################################
//#################################### MAIN LOOP ##############################################
//#############################################################################################


void jogpad_check() {
	uint8_t buttons_temp = 0;
  uint8_t idx;
	set_led_status();
  spi_txrx_inout();	// Alle SPI-IOs holen/setzen
	

#ifndef JOGPAD
	return;
#endif

  plan_line_data_t plan_data;  // Initialize planner data struct for motion blocks.
  plan_line_data_t *pl_data = &plan_data;

// ################## M A C H I N E   B U T T O N S #########################

// MACHINE_INP_SR: Buttons, wartet auf Loslassen
// ---7---	---6---	 ---5---	---4---  ---3---  ---2---  ---1---  ---0---	
//  HOME    CLR_ALRM                   ZERO_ALL  ZERO_Z   ZERO_Y  ZERO_X

	buttons_temp = MACHINE_INP_SR; 	// unbenutzte Buttons könnten hier ausmaskiert werden
	if (buttons_temp) {
	  // Einer der Buttons an MACHINE_INP_SR gedrückt	
  	if (buttons_temp & (1<<HOME_SW)) {	
		  if (bit_istrue(settings.flags,BITFLAG_HOMING_ENABLE)) {
	  	  spindle_stop(); 
		    sys.state = STATE_HOMING;
	    	set_led_status();
   			spi_txrx_inout();
 				sr_dispstate_1 = sys.state; // Homing-Status auch an Display
				spi_tx_axis(0); 						// irgendeine Achse, hier X 
		    // Only perform homing if Grbl is idle or lost.	    
		    mc_homing_cycle(HOMING_CYCLE_ALL); 
		    if (!sys.abort) {  // Execute startup scripts after successful homing.
		      sys.state = STATE_IDLE; // Set to IDLE when complete.
		      st_go_idle(); // Set steppers to the settings idle state before returning.
		    	}	  	
		  	} else {
		    	report_feedback_message(MESSAGE_DISABLED);
	  	}
  	}
  	// jog_zero_request_flag wird in report_realtime_status() zurückgesetzt
    // WPos = MPos - WCS - G92 - TLO  ->  G92 = MPos - WCS - TLO - WPos
    // Berechnung: wco[idx] = gc_state.coord_system[idx] + gc_state.coord_offset[idx];
  	if (buttons_temp & (1<<ZERO_ALL_SW)) {	
			jogpad_zero_all();
      #ifdef ZERO_MSG
		    printPgmString(PSTR("[MSG:ZeroAll,PANEL]\r\n"));
	  	#else
	  	  return;  // repeat until released
	  	#endif
  	}
  	if (buttons_temp & (1<<ZERO_Z_SW)) {	
			jog_zero_request_flag = 4;
      gc_state.coord_offset[2] = gc_state.position[2];
      system_flag_wco_change(); // Set to refresh immediately, something altered.
      #ifdef ZERO_MSG
		    printPgmString(PSTR("[MSG:ZeroZ,PANEL]\r\n"));
	  	#else
	  	  return;  // repeat until released
	  	#endif
  	}
  	if (buttons_temp & (1<<ZERO_Y_SW)) {	
			jog_zero_request_flag = 2;
      gc_state.coord_offset[1] = gc_state.position[1];
      system_flag_wco_change(); // Set to refresh immediately, something altered.
      #ifdef ZERO_MSG
		    printPgmString(PSTR("[MSG:ZeroY,PANEL]\r\n"));
	  	#else
	  	  return;  // repeat until released
	  	#endif
  	}
  	if (buttons_temp & (1<<ZERO_X_SW)) {	
			jog_zero_request_flag = 1;
      gc_state.coord_offset[0] = gc_state.position[0];
      system_flag_wco_change(); // Set to refresh immediately, something altered.
      #ifdef ZERO_MSG
		    printPgmString(PSTR("[MSG:ZeroX,PANEL]\r\n"));
	  	#else
	  	  return;  // repeat until released
	  	#endif
  	}
  	if (buttons_temp & (1<<CLEAR_ALARM_SW)) {	
  		report_feedback_message(MESSAGE_ALARM_UNLOCK);
	    system_clear_exec_alarm(); // Clear alarm
	    sys.state = STATE_IDLE;
  	}

  	if (buttons_temp & (1<<FEED_FASTER_SW)) {
			system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS);
  	}	

  	if (buttons_temp & (1<<FEED_SLOWER_SW)) {
			system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS);
  	}	
	  machine_btn_release();
	  return;
	}
 	
// ############### A C C E S S O R Y   B U T T O N S #####################

	buttons_temp = ACCESSORY_INP_SR; 	// unbenutzte Buttons könnten hier ausmaskiert werden
	if (buttons_temp) {
  	if (buttons_temp & (1<<FLOOD_ON_SW)) {
	    coolant_set_state(COOLANT_FLOOD_ENABLE);
  	}	
  	if (buttons_temp & (1<<MIST_ON_SW)) {
	    coolant_set_state(COOLANT_MIST_ENABLE);
  	}	
  	if (buttons_temp & (1<<FLOOD_MIST_OFF_SW)) {
	    coolant_set_state(COOLANT_DISABLE);
  	}
  	if (buttons_temp & (1<<ATC_ON_SW)) {
	    coolant_set_state(ATC_ENABLE);
  	}	 		
  	if (buttons_temp & (1<<AUX1_ON_SW)) {
	    coolant_set_state(AUX1_ENABLE);
  	}	 		
  	if (buttons_temp & (1<<AUX2_ON_SW)) {
	    coolant_set_state(AUX2_ENABLE);
  	}	 		
  	if (buttons_temp & (1<<AUX3_ON_SW)) {
	    coolant_set_state(AUX3_ENABLE);
  	}	 		
  	if (buttons_temp & (1<<ATC_AUX_OFF_SW)) {
	    coolant_set_state(ATC_DISABLE);
	    coolant_set_state(AUX1_DISABLE);
	    coolant_set_state(AUX2_DISABLE);
	    coolant_set_state(AUX3_DISABLE);
  	}	
	  accessory_btn_release();
	  return;
	}
	if (sys.state != STATE_IDLE) return;   // Alles weitere nur wenn IDLE!

	float max_travel = 0.0;

// ######################## J O Y S T I C K ###############################

// JOY_INP_SR: Momentan-Schalter vom Joystick, SR auf Erweiterung
// ---7---	---6--- 	---5--- 	---4--- 	---3---	 ---2---	---1---	 ---0---	
//          ZERO_JOY   REV_Z     REV_Y     REV_X    FWD_Z    FWD_Y    FWD_X  

  #ifdef ANALOG_JOYSTICK
	  buttons_temp = JOY_INP_SR; 	// unbenutzte Buttons könnten hier ausmaskiert werden
		float joy_target_mpos[N_AXIS];    	// Maschinenposition in mm
			
		uint8_t joy_axis = 255;	// kein Dial-Schalter EIN
		uint8_t joy_rev = 0;
		old_f_override = sys.f_override;
		
		
		if (buttons_temp) {
      memset(pl_data,0,sizeof(plan_line_data_t)); // Zero pl_data struct
			// irgendein Joystick-Button?
	    for (idx=0; idx < N_AXIS; idx++) {
	    	joy_target_mpos[idx] = gc_state.position[idx]; // default-Target
			}
			if (buttons_temp & (1<<FWD_X_SW)) {
				joy_axis = 0;
			} else if (buttons_temp & (1<<FWD_Y_SW)) {	
				joy_axis = 1;
			} else if (buttons_temp & (1<<FWD_Z_SW)) {	
				joy_axis = 2;
			}
			if (buttons_temp & (1<<REV_X_SW)) {
				joy_axis = 0;
				joy_rev = 1;
				joy_target_mpos[0] = 0;
			} else if (buttons_temp & (1<<REV_Y_SW)) {	
				joy_axis = 1;
				joy_rev = 1;
				joy_target_mpos[1] = 0;
			} else if (buttons_temp & (1<<REV_Z_SW)) {	
				joy_axis = 2;
				joy_rev = 1;
				joy_target_mpos[2] = 0;
			}
			if (buttons_temp & (1<<JOY_ZERO_SW)) {
				jogpad_zero_all();
	      #ifdef ZERO_MSG
			    printPgmString(PSTR("[MSG:ZeroAll,JOYSTICK]\r\n"));
			    joy_btn_release();
			    return;
		  	#else
		  	  return;  // repeat until released
		  	#endif
			}
			// Bewegungs-Vektor auf Maschinengröße begrenzen; kann negativ sein!
			if (joy_rev) {
				max_travel = min(max_travel,(-settings.max_travel[joy_axis]));
			} else {
				max_travel = max(max_travel,(-settings.max_travel[joy_axis]));
			}
      memcpy(joy_target_mpos,gc_state.position,sizeof(gc_state.position));
			joy_target_mpos[joy_axis] = max_travel;
			
			adc_raw = ADCH;
		  adc_squared = (float)adc_raw;
 		  adc_squared += JOY_ADC_OFFS;
 			adc_squared = adc_squared * adc_squared / 325; // soll maximal 200 Prozent erreichen
			adc_dest_feed = max(1, round(adc_squared));
			adc_current_feed = adc_dest_feed;
			
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:JOY START "));
		    serial_write(88 + joy_axis);
		    serial_write(':');
		    printFloat_CoordValue(max_travel);
		  	printPgmString(PSTR("]\r\n"));
		    printPgmString(PSTR("[MSG:JOY F OVR:"));
		    printInteger(adc_current_feed);
		  	printPgmString(PSTR("]\r\n"));
			#endif
			
	  	pl_data->feed_rate = settings.max_rate[joy_axis] / 2; // half max due to 200% override
	  	mc_line(joy_target_mpos, pl_data); 
		  if (sys.state == STATE_IDLE) {
		    if (plan_get_current_block() != NULL) { // Check if there is a block to execute.
		      sys.state = STATE_JOG;
	      	set_led_status();
		      st_prep_buffer();
		      sys.f_override = adc_current_feed;
		      plan_update_velocity_profile_parameters();
		      plan_cycle_reinitialize();
		      st_wake_up();  // NOTE: Manual start. No state machine required.
		    }
		  }
	    
			uint8_t buttons_temp_old = buttons_temp;

			do {
				spi_txrx_inout();
	    	buttons_temp = JOY_INP_SR;
	    		
				spi_tx_axis(joy_axis);  	    // neuen Status an Display
    		protocol_execute_realtime();  // hält die Maschine am Laufen
    		
				adc_raw = ADCH;
  		  adc_squared = (float)adc_raw;
  		  adc_squared += JOY_ADC_OFFS;
	 			adc_squared = adc_squared * adc_squared / 325; // soll maximal 200 Prozent erreichen
				adc_dest_feed = max(1, round(adc_squared));

				bool update_ovr = false;
				if (adc_current_feed > adc_dest_feed) { 
					adc_current_feed--;
				  update_ovr = true;
				}
				if (adc_dest_feed > adc_current_feed) {
					adc_current_feed++;
				  update_ovr = true;
				}
			  if (update_ovr) {
		      sys.f_override = adc_current_feed;
		      plan_update_velocity_profile_parameters();
		      plan_cycle_reinitialize();
					#ifdef debug_jog
				    printPgmString(PSTR("[MSG:JOY F OVR:"));
				    printInteger(adc_current_feed);
				  	printPgmString(PSTR("]\r\n"));
					#endif
				  
				} 
				delay_ms(3);
		} while (buttons_temp_old == buttons_temp);
		
    if (sys.state & STATE_JOG) { 
    	// Wichtig: Führt sonst zum Absturz, wenn Ende bereits erreicht:
    	// Block all other states from invoking motion cancel.
    	system_set_exec_state_flag(EXEC_MOTION_CANCEL);
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:JOY CANCEL]\r\n"));
			#endif
    }
		sys.f_override = old_f_override;
   	system_clear_exec_motion_overrides(); 
		#ifdef debug_jog
	    printPgmString(PSTR("[MSG:JOY OVR CANCEL]\r\n"));
		#endif
		plan_sync_position();
		//protocol_buffer_synchronize();
 	} // sr_inputs active
	#endif
	
// ############################ D I A L ##############################
	
	// DIAL_INP_SR: Schalter DIAL-Richtung
	// ---7---	---6--- 	---5--- 	---4--- 	---3---	 ---2---	---1---	 ---0---	
	//                                          FAST    DIR_Z    DIR_Y    DIR_X  
  buttons_temp = DIAL_INP_SR;
	if (buttons_temp & (1<<DIAL_ZERO_SW)) {
		jogpad_zero_all();
    #ifdef ZERO_MSG
	    printPgmString(PSTR("[MSG:ZeroAll,DIAL]\r\n"));
	    dial_btn_release();
	    return;
  	#else
  	  return;  // repeat until released
  	#endif
	}

	uint8_t dial_fast;
	float my_pos;
	
	if (buttons_temp & (1<<DIAL_DIR_X_SW)) {	
		dial_axis = 0;
	} else if (buttons_temp & (1<<DIAL_DIR_Y_SW)) {	
		dial_axis = 1;
	} else if (buttons_temp & (1<<DIAL_DIR_Z_SW)) {	
		dial_axis = 2;
	}
	dial_fast = bit_istrue(buttons_temp, bit(DIAL_FAST_SW));

  if (dial_sema || (dial_delta_32 != 0)) { 
	// Impuls(e) vom Handrad empfangen. Zielposition aus Delta-Wert dial_delta_32 errechnen 
	// und Ziel über mc_line(target) anfahren.

     // nur im Ruhezustand oder bei Jog annehmen, sonst dial_delta_32 verwerfen
    if (sys.state != STATE_IDLE && sys.state != STATE_JOG) { 
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:DIAL NOT_IDLE"));
		  	printPgmString(PSTR("]\r\n"));
    		dial_delta_32 = 0;
			#endif
			return;
		}		   
	  
	  // transfer dial deltafrom ISR to non-volatile dial_delta_32
  	uint8_t sreg = SREG;
  	cli();              // prevent pin change ISR
  	dial_sema = false;  
  	dial_delta_32 += (int32_t)dial_delta_8;
   	dial_delta_8 = 0;	  // we used it, so reset
 		SREG = sreg;        // restore old interrupt status
 		
    // if buffer is full, keep new dial_delta_32 and return
	  if (plan_check_full_buffer()) {
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:DIAL BUF_FULL"));
		  	printPgmString(PSTR("]\r\n"));
			#endif
			return;		   
	  }
 		
    if (sys.state == STATE_JOG) return; // perevent new pos. issued before previous done. Keep dial_delta_32.

    memset(pl_data,0,sizeof(plan_line_data_t)); // Zero pl_data struct
    if (dial_fast) {
      dial_delta_f = (float)dial_delta_32 * DIAL_MM_COARSE;
	  	pl_data->feed_rate = settings.max_rate[dial_axis] * DIAL_FAST_FAC; // fast move
    } else {
      dial_delta_f = (float)dial_delta_32 * DIAL_MM_COARSE;
	  	pl_data->feed_rate = settings.max_rate[dial_axis] * DIAL_SLOW_FAC;
    }
 		dial_delta_32 = 0;  // had been transferred, reset
    
		// new target accepted, issue move. We use gc_state.position so no sync is necessary (I hope!)
		my_pos = gc_state.position[dial_axis] + dial_delta_f; // update selected axis pos.   
		if (dial_delta_f < 0) {
			max_travel = min(max_travel,(-settings.max_travel[dial_axis]));
			my_pos = max(max_travel, my_pos);
		} else {
			max_travel = max(max_travel,(-settings.max_travel[dial_axis]));
			my_pos = min(max_travel, my_pos);
		}
    gc_state.position[dial_axis] = my_pos;
    spi_txrx_inout();               // update status LEDs, JOG ON
    mc_line(gc_state.position, pl_data); 
    if (sys.state == STATE_IDLE) { 
			if (plan_get_current_block() != NULL) { // Check if there is a block to execute.
				sys.state = STATE_JOG;
        st_prep_buffer();
        st_wake_up();  // NOTE: Manual start. No state machine required.
			}
		}
	  
/*
		  
*/
		#ifdef debug_jog
	    printPgmString(PSTR("[MSG:DIAL_"));
	    serial_write(88 + dial_axis);
	    serial_write(" DELTA:");
	    printFloat_CoordValue(dial_delta_f);
	  	printPgmString(PSTR("]\r\n"));
		#endif
		// plan_sync_position();
		// protocol_buffer_synchronize();
  } // dial_sema
}