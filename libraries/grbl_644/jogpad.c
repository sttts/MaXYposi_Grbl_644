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


uint8_t old_f_override;

volatile int8_t last_pin, last_pin0, last_pin1;		// benutzt in ISR(DIAL_INT_vect)
volatile bool dial_sema = false;
volatile int8_t dial_delta_8;      // Schritte von Handrad aus ISR(DIAL_INT_vect), Signed Byte!

uint8_t dial_fast = 0;
float dial_delta_f = 0.0;       // Akkumulator für Handrad-Werte
int32_t dial_delta_32 = 0;	           // Akkumulator für Handrad-Werte
uint8_t dial_axis = 0;

uint16_t blink_count = 0; 
uint16_t activity_count = 0;
uint8_t blink_toggle;

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
		#ifdef debug_dial_isr
			// Show state sequence of wheel:
	    // serial_write(pin + 48);
		  serial_write('*');
		  print_uint8_base10(dial_delta_8);
		  printPgmString(PSTR("\r\n"));
		  serial_write('*');
		#endif
	}
}


uint8_t adc_raw, adc_dest_feed, adc_current_feed; // ADC value

uint8_t get_feed_from_adc() {
	
	uint8_t adc_raw; // ADC value
	float adc_squared;

	adc_raw = ADCH;
  adc_squared = (float)adc_raw;
  adc_squared += JOY_ADC_OFFS;
	adc_squared = adc_squared * adc_squared / 325; // soll maximal 200 Prozent erreichen
	return(max(1, round(adc_squared)));
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
	LIMIT_DDR &= ~(DIAL_MASK); // Set as input pins
  LIMIT_PORT |= DIAL_MASK; // Active low operation. Pull-up on.
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
	spindle_on = false;
	flood_on = false;
	mist_on = false;
	atc_on = false;
	aux1_on = false;
	aux2_on = false;
	aux3_on = false;
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
  #ifdef LED_JOG
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
  
  #ifdef LED_FLOOD
		if (flood_on) {
			leds_temp |= (1 << LED_FLOOD);
		}
	#endif;
	#ifdef LED_MIST
		if (mist_on) {
			leds_temp |= (1 << LED_MIST);
		}
	#endif;
	#ifdef DIAL	
		if (dial_axis == 0) {
			#ifdef LED_DIAL_SELECT_X
				leds_temp |= (1 << LED_DIAL_SELECT_X);
			#endif
		} else if (dial_axis == 1) {
			#ifdef LED_DIAL_SELECT_X
				leds_temp |= (1 << LED_DIAL_SELECT_Y);
			#endif
		} else if (dial_axis == 2) {
			#ifdef LED_DIAL_SELECT_X
				leds_temp |= (1 << LED_DIAL_SELECT_Z);
			#endif
		} else if (dial_axis == 3) {
			#ifdef LED_DIAL_SELECT_X
				leds_temp |= (1 << LED_DIAL_SELECT_C);
			#endif
		}
	  #ifdef LED_DIAL_FAST
		  if (dial_fast) {
				leds_temp |= (1 << LED_DIAL_FAST);
			}
		#endif
	#endif;
 	STATUS_LED_OUT_SR = leds_temp; 		// Status-LED-Port setzen
 	#ifdef SPI_DISP	
 		sr_dispstate_0 = MACHINE_OUT_SR; 	// wurde in spindle/coolant_control gesetzt
	#endif;
}

void btn_wait_execute() {
	set_led_status();
  spi_txrx_inout();
  delay_ms(3);
	spi_tx_axis(0);
  protocol_execute_realtime(); 
  delay_ms(3);
	spi_tx_axis(1);
  protocol_execute_realtime(); 
  delay_ms(3);
	spi_tx_axis(2);
  #ifdef AXIS_C_ENABLE
	  protocol_execute_realtime(); 
	  delay_ms(3);
		spi_tx_axis(3);
	#endif
  delay_ms(3);
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

void dial_btn_release() {
  while (DIAL_INP_SR) { 		// until released
    btn_wait_execute(); 
  }
} 

void jogpad_zero_all() {
	uint8_t idx;
	jog_zero_request_flag = 15;
  for (idx=0; idx<N_AXIS; idx++) {
    gc_state.coord_offset[idx] = gc_state.position[idx];
	}
  system_flag_wco_change(); // Set to refresh immediately, something altered.
}



//#############################################################################################
//#################################### MAIN LOOP ##############################################
//#############################################################################################


void jogpad_check() {
	set_led_status();
  spi_txrx_inout();	// Alle SPI-IOs holen/setzen
	
#ifndef JOGPAD
	return;
#endif

	uint8_t buttons_temp = 0;
  uint8_t idx;
	uint8_t temp_state;
  plan_line_data_t plan_data;  // Initialize planner data struct for motion blocks.
  plan_line_data_t *pl_data = &plan_data;

// #######################################################################
// ################# M A C H I N E   B U T T O N S #######################
// #######################################################################

	buttons_temp = MACHINE_INP_SR; 	// unbenutzte Buttons könnten hier ausmaskiert werden
	if (buttons_temp) {
		delay_ms(10);
    spi_txrx_inout();	// Entprellung: Alle SPI-IOs nochmals holen/setzen
		buttons_temp = MACHINE_INP_SR;
		if (buttons_temp == 0) return;
	  // Einer der Buttons an MACHINE_INP_SR gedrückt	
  	if ((buttons_temp & (1<<HOME_SW)) && ((sys.state == STATE_ALARM) || (sys.state == STATE_IDLE))) {	
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
  	if ((buttons_temp & (1<<CLEAR_ALARM_SW)) && (sys.state == STATE_ALARM)) {	
  		report_feedback_message(MESSAGE_ALARM_UNLOCK);
	    system_clear_exec_alarm(); // Clear alarm
	    sys.state = STATE_IDLE;
  	}
  	if (sys.state == STATE_IDLE) {
	  	// jog_zero_request_flag wird in report_realtime_status() zurückgesetzt
	    // WPos = MPos - WCS - G92 - TLO  ->  G92 = MPos - WCS - TLO - WPos
	    // Berechnung: wco[idx] = gc_state.coord_system[idx] + gc_state.coord_offset[idx];
			#ifdef AXIS_C_ENABLE
		  	#ifdef ZERO_C_SW
			  	if (buttons_temp & (1<<ZERO_C_SW)) {	
						jog_zero_request_flag = 8;
			      gc_state.coord_offset[3] = gc_state.position[3];
			      system_flag_wco_change(); // Set to refresh immediately, something altered.
			      #ifdef ZERO_MSG
					    printPgmString(PSTR("[MSG:BTN ZeroC]\r\n"));
				  	#else
				  	  return;  // repeat until released
				  	#endif
			  	}
		  	#endif
	  	#endif
	  	#ifdef ZERO_ALL_SW
		  	if (buttons_temp & (1<<ZERO_ALL_SW)) {	
					jogpad_zero_all();
		      #ifdef ZERO_MSG
				    printPgmString(PSTR("[MSG:BTN ZeroAll]\r\n"));
			  	#else
			  	  return;  // repeat until released
			  	#endif
		  	}
	  	#endif
	  	#ifdef ZERO_Z_SW
		  	if (buttons_temp & (1<<ZERO_Z_SW)) {	
					jog_zero_request_flag = 4;
		      gc_state.coord_offset[2] = gc_state.position[2];
		      system_flag_wco_change(); // Set to refresh immediately, something altered.
		      #ifdef ZERO_MSG
				    printPgmString(PSTR("[MSG:BTN ZeroZ]\r\n"));
			  	#else
			  	  return;  // repeat until released
			  	#endif
		  	}
	  	#endif
	  	#ifdef ZERO_Y_SW
		  	if (buttons_temp & (1<<ZERO_Y_SW)) {	
					jog_zero_request_flag = 2;
		      gc_state.coord_offset[1] = gc_state.position[1];
		      system_flag_wco_change(); // Set to refresh immediately, something altered.
		      #ifdef ZERO_MSG
				    printPgmString(PSTR("[MSG:BTN ZeroY]\r\n"));
			  	#else
			  	  return;  // repeat until released
			  	#endif
		  	}
	  	#endif
	  	#ifdef ZERO_X_SW
		  	if (buttons_temp & (1<<ZERO_X_SW)) {	
					jog_zero_request_flag = 1;
		      gc_state.coord_offset[0] = gc_state.position[0];
		      system_flag_wco_change(); // Set to refresh immediately, something altered.
		      #ifdef ZERO_MSG
				    printPgmString(PSTR("[MSG:BTN ZeroX]\r\n"));
			  	#else
			  	  return;  // repeat until released
			  	#endif
		  	}
	  	#endif
	  }

	  #ifdef FEED_FASTER_SW
	  	if (buttons_temp & (1<<FEED_FASTER_SW)) {
				system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS);
	  	}	
  	#endif
	  #ifdef FEED_SLOWER_SW
	  	if (buttons_temp & (1<<FEED_SLOWER_SW)) {
				system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS);
	  	}	
  	#endif
	  machine_btn_release();
	  return;
	}
 	
// #######################################################################
// ############### A C C E S S O R Y   B U T T O N S #####################
// #######################################################################

	buttons_temp = ACCESSORY_INP_SR;
	if (buttons_temp) {
		delay_ms(10);
    spi_txrx_inout();	// Entprellung: Alle SPI-IOs nochmals holen/setzen
		buttons_temp = ACCESSORY_INP_SR;
		if (buttons_temp == 0) return;
		
  	if (buttons_temp & (1<<FLOOD_ON_SW)) {
  		system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE);
      protocol_execute_realtime();
   	}	
  	if (buttons_temp & (1<<MIST_ON_SW)) {
  		system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE);
      protocol_execute_realtime();
   	}	
  	  	
  	if (buttons_temp & (1<<ATC_ON_SW)) {
  		if (atc_on) {
 	    	coolant_set_state(ATC_DISABLE);
 	    	gc_state.modal.coolant |= (1<<COOLANT_STATE_ATC);
 		} else {
	    	coolant_set_state(ATC_ENABLE);
 	    	gc_state.modal.coolant &= ~(1<<COOLANT_STATE_ATC);
	    }
  	}	 		
  	if (buttons_temp & (1<<AUX1_ON_SW)) {
  		if (aux1_on) {
	    	coolant_set_state(AUX1_DISABLE);
  		} else {
	    	coolant_set_state(AUX1_ENABLE);
	  	}
  	}	 		
  	if (buttons_temp & (1<<AUX2_ON_SW)) {
  		if (aux2_on) {
	    	coolant_set_state(AUX2_DISABLE);
  		} else {
	    	coolant_set_state(AUX2_ENABLE);
	  	}
  	}	 		
  	if (buttons_temp & (1<<AUX3_ON_SW)) {
  		if (aux3_on) {
	    	coolant_set_state(AUX3_DISABLE);
  		} else {
	    	coolant_set_state(AUX3_ENABLE);
	  	}
  	}	 		
  	if (buttons_temp & spindle_on & (1<<SPINDLE_FASTER_SW)) {
			#ifdef VARIABLE_SPINDLE
      	system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS);
      	protocol_execute_realtime();
			  spindle_sync(gc_state.modal.spindle,settings.rpm_max);
//			  spindle_set_speed(spindle_compute_pwm_value(gc_state.spindle_speed));
			#endif
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:BTN SPINDLE OVR PLUS]\r\n"));
			#endif
  	}	
  	if (buttons_temp  & spindle_on & (1<<SPINDLE_SLOWER_SW)) {
			#ifdef VARIABLE_SPINDLE
        system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS);
      	protocol_execute_realtime();
			  spindle_sync(gc_state.modal.spindle,settings.rpm_max);
			#endif
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:BTN SPINDLE OVR MINUS]\r\n"));
			#endif
	  }	
		accessory_btn_release();
		return;
	}
	
// #######################################################################
// ###################### D I A L   B U T T O N S ########################
// #######################################################################

	if (sys.state != STATE_IDLE) return;   // Alles weitere nur wenn IDLE!

  buttons_temp = DIAL_INP_SR;

//	dial_fast = buttons_temp & (1<<DIAL_FAST_SW);
//	buttons_temp &= ~(1<<DIAL_FAST_SW);	// falls Dauer-Button oder Schalter!
	if (buttons_temp) {
		delay_ms(10);
    spi_txrx_inout();	// Entprellung: Alle SPI-IOs nochmals holen/setzen
//	buttons_temp &= ~(1<<DIAL_FAST_SW);	// falls Dauer-Button oder Schalter!
		buttons_temp = DIAL_INP_SR;
		if (buttons_temp == 0) return;
	
		if (buttons_temp & (1<<DIAL_ZERO_SW)) {
			jogpad_zero_all();
	    #ifdef ZERO_MSG
		    printPgmString(PSTR("[MSG:BTN ZeroAll]\r\n"));
		    dial_btn_release();
	  	#else
	 	  	return;  // repeat until released
	  	#endif
		}
		
		if (buttons_temp & (1<<DIAL_DIR_X_SW)) {	
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:BTN DIAL AXIS X]\r\n"));
			#endif
			dial_axis = 0;
		} else if (buttons_temp & (1<<DIAL_DIR_Y_SW)) {	
			dial_axis = 1;
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:BTN DIAL AXIS Y]\r\n"));
			#endif
		} else if (buttons_temp & (1<<DIAL_DIR_Z_SW)) {	
			dial_axis = 2;
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:BTN DIAL AXIS Z]\r\n"));
			#endif
		#ifdef AXIS_C_ENABLE
			} else if (buttons_temp & (1<<DIAL_DIR_C_SW)) {	
				dial_axis = 3;
				#ifdef debug_jog
			    printPgmString(PSTR("[MSG:BTN DIAL AXIS C]\r\n"));
				#endif
		#endif
		}
		
  	if (buttons_temp & (1<<DIAL_FAST_SW)) {
		  dial_fast = ~dial_fast;
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:BTN DIAL FAST]\r\n"));
			#endif
		}

  	if (buttons_temp & (1<<DIAL_SPINDLE_TOGGLE)) {
      if (spindle_on) {
	    	spindle_sync(SPINDLE_DISABLE,0.0);
      	gc_state.modal.spindle = SPINDLE_DISABLE;
				#ifdef debug_jog
			    printPgmString(PSTR("[MSG:BTN SPINDLE STOP]\r\n"));
				#endif
      } else {
      	gc_state.spindle_speed = settings.rpm_max;
      	gc_state.modal.spindle = SPINDLE_ENABLE_CW;
	    	///system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET);
	    	sys.spindle_speed_ovr = 50.0;
        sys.report_ovr_counter = 0; // Set to report change immediately 
      	//protocol_execute_realtime();
	    	spindle_sync(SPINDLE_ENABLE_CW,settings.rpm_max);
	    	
				#ifdef debug_jog
			    printPgmString(PSTR("[MSG:BTN SPINDLE ON HALF]\r\n"));
				#endif
	  	}
	  }
	  dial_btn_release();
	  return;
	}

	float max_travel = 0.0;

// #######################################################################
// ######################### J O Y S T I C K #############################
// #######################################################################

// JOY_INP_SR: Momentan-Schalter vom Joystick, SR auf Erweiterung

  #ifdef ANALOG_JOYSTICK
	  buttons_temp = JOY_INP_SR; 	// unbenutzte Buttons könnten hier ausmaskiert werden
		float joy_target_mpos[N_AXIS];    	// Maschinenposition in mm
			
		uint8_t joy_axis;	// kein Dial-Schalter EIN
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
			#ifdef AXIS_C_ENABLE
				} else if (buttons_temp & (1<<FWD_C_SW)) {	
					joy_axis = 3;
			#endif
			}
			if (buttons_temp & (1<<REV_X_SW)) {
				joy_axis = 0;
				joy_rev = 1;
			} else if (buttons_temp & (1<<REV_Y_SW)) {	
				joy_axis = 1;
				joy_rev = 1;
			} else if (buttons_temp & (1<<REV_Z_SW)) {	
				joy_axis = 2;
				joy_rev = 1;
			#ifdef AXIS_C_ENABLE
				} else if (buttons_temp & (1<<REV_C_SW)) {	
					joy_axis = 3;
					joy_rev = 1;
			#endif
			}
			// Bewegungs-Vektor auf Maschinengröße begrenzen; kann negativ sein!
			#ifdef HOMING_FORCE_SET_ORIGIN
				if (joy_axis == 2) {		// Negative Z-Richtung!
					if (joy_rev) {
						max_travel = min(max_travel,(settings.max_travel[joy_axis]));
					} else {
						max_travel = max(max_travel,(settings.max_travel[joy_axis]));
					}
				} else {
					if (joy_rev) {
						max_travel = min(max_travel,(-settings.max_travel[joy_axis]));
					} else {
						max_travel = max(max_travel,(-settings.max_travel[joy_axis]));
					}
				}
			#else
				if (joy_axis == 2) {		// Negative Z-Richtung!
					if (joy_rev) {
						max_travel = min(max_travel,(settings.max_travel[joy_axis]));
					} else {
						max_travel = max(max_travel,(settings.max_travel[joy_axis]));
					}
				} else {
					if (joy_rev) {
						max_travel = min(max_travel,(-settings.max_travel[joy_axis]));
					} else {
						max_travel = max(max_travel,(-settings.max_travel[joy_axis]));
					}
				}
			#endif
      memcpy(joy_target_mpos,gc_state.position,sizeof(gc_state.position));
			joy_target_mpos[joy_axis] = max_travel;
			
			adc_dest_feed = get_feed_from_adc();
			adc_current_feed = 1;
		  sys.f_override = 1;
			
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:JOY START "));
		    if (joy_axis == 3) {
		    	serial_write('C');
		    } else {
		    	serial_write(88 + joy_axis);
		    }
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
    		
			  adc_dest_feed = get_feed_from_adc();

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
		gc_state.position[joy_axis] = joy_target_mpos[joy_axis];
		//protocol_buffer_synchronize();
 	} // sr_inputs active
	#endif
	
// #######################################################################
// ############################## D I A L ################################
// #######################################################################
	
	float my_pos;

  if (dial_sema || (dial_delta_32 != 0)) { 
	// Impuls(e) vom Handrad empfangen. Zielposition aus Delta-Wert dial_delta_32 errechnen 
	// und Ziel über mc_line(target) anfahren.
		#ifdef debug_jog
		  if (dial_fast) {
	    	printPgmString(PSTR("[MSG:DIAL FAST]\r\n"));
	    } else {
	    	printPgmString(PSTR("[MSG:DIAL SLOW]\r\n"));
	    }
		#endif

     // nur im Ruhezustand oder bei Jog annehmen, sonst dial_delta_32 verwerfen
    if (sys.state != STATE_IDLE && sys.state != STATE_JOG) { 
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:DIAL NOT IDLE"));
		  	printPgmString(PSTR("]\r\n"));
			#endif
   		dial_delta_32 = 0;
			return;
		}		   
	  
	  // transfer dial delta from ISR to non-volatile dial_delta_32
  	uint8_t sreg = SREG;
  	cli();              // prevent pin change ISR
  	dial_sema = false;  
  	dial_delta_32 += (int32_t)dial_delta_8;
   	dial_delta_8 = 0;	  // we used it, so reset
 		SREG = sreg;        // restore old interrupt status
 		
    // if buffer is full, keep new dial_delta_32 and return
	  if (plan_check_full_buffer()) {
			#ifdef debug_jog
		    printPgmString(PSTR("[MSG:DIAL BUFFER FULL"));
		  	printPgmString(PSTR("]\r\n"));
			#endif
			return;		   
	  }
 		
    if (sys.state == STATE_JOG) return; // perevent new pos. issued before previous done. Keep dial_delta_32.

    memset(pl_data,0,sizeof(plan_line_data_t)); // Zero pl_data struct
    if (dial_axis == 3)  {
	    if (dial_fast) {
	      dial_delta_f = (float)dial_delta_32 * DIAL_DEG_COARSE;
		  	pl_data->feed_rate = settings.max_rate[dial_axis] * DIAL_FAST_FAC; // fast move
	    } else {
	      dial_delta_f = (float)dial_delta_32 * DIAL_DEG_FINE;
		  	pl_data->feed_rate = settings.max_rate[dial_axis] * DIAL_SLOW_FAC;
	    }
    } else {
	    if (dial_fast) {
	      dial_delta_f = (float)dial_delta_32 * DIAL_MM_COARSE;
		  	pl_data->feed_rate = settings.max_rate[dial_axis] * DIAL_FAST_FAC; // fast move
	    } else {
	      dial_delta_f = (float)dial_delta_32 * DIAL_MM_FINE;
		  	pl_data->feed_rate = settings.max_rate[dial_axis] * DIAL_SLOW_FAC;
	    }
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