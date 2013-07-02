
void check_checksum(void)
{
	uint8_t dummy, i;
	
  checksum_OK = 0;
  checksum = ascii_to_dec(command_received);
  dummy = 0;
  
	for (i=1; i<command_received; i++)
		dummy += Rx_buffer[i];
  if (checksum == dummy)
    checksum_OK = 1;
} /* c h e c k _ c h e c k s u m */



void calculate_checksum(uint8_t length)
{
	uint8_t i;

  checksum = 0;
	for (i=1; i<=length; i++)
		checksum += Tx_buffer[i];
} /* c a l c u l a t e _ c h e c k s u m */


uint8_t ascii_to_dec(uint8_t idx)
{
	uint8_t temp1, temp2;
	
  temp1 = Rx_buffer[idx];
  if (temp1 <= '9')
    temp1 -= 48;
  else
    temp1 -= 55;
  temp1 *= 16;
  temp2 = Rx_buffer[idx+1];
  if (temp2 <= '9')
    temp2 -= 48;
  else
    temp2 -= 55;
  temp1 += temp2;
  return (temp1);      
} /* a s c i i _ t o _ d e c */



void dec_to_ascii(uint8_t idx, uint8_t seal_parameter)
{
	uint8_t temp1;
	
	temp1 = seal_parameter / 16;
	if (temp1 < 10)
	  temp1 += 48;
	else
	  temp1 += 55;
	Tx_buffer[idx] = temp1;

	temp1 = seal_parameter % 16;
	if (temp1 < 10)
	  temp1 += 48;
	else
	  temp1 += 55;
	Tx_buffer[idx+1] = temp1;
} /* d e c _ t o _ a s c i i */



void transmit_seal_program(void)
{
	Tx_buffer[0] = '[';
	Tx_buffer[1] = '0';                                      // command number = 01
	Tx_buffer[2] = '1';

  Tx_data = read_EE_byte(EE_SEAL_TEMP);                    // seal temp
  dec_to_ascii(3, Tx_data);

  Tx_data = read_EE_byte(EE_BLOWER_SPEED);                 // blower speed
  dec_to_ascii(5, Tx_data);

  Tx_data = read_EE_byte(EE_ASSEMBLY_STEPS);               // number of steps to close the seal assembly
  dec_to_ascii(7, Tx_data);

  Tx_data = read_EE_byte(EE_DRIVER_DELAY);                 // driver motor delay
  dec_to_ascii(9, Tx_data);

  calculate_checksum(10);
  dec_to_ascii(11, checksum);
	Tx_buffer[13] = ']';
  command_transmitted = 0;
	Tx_index = 1;
  UDR0 = '[';                                               // start transmission
} /* t r a n s m i t _ s e a l _ p r o g r a m */



void transmit_settings(void)
{
	Tx_buffer[0] = '[';
	Tx_buffer[1] = '0';                            // command number = 03
	Tx_buffer[2] = '3';
  dec_to_ascii(3, temp_offset-TEMP_OFFSET_MIDLE);
  dec_to_ascii(5, blower_offset-blower_offset_middle_var);
  dec_to_ascii(7, required_length);
  dec_to_ascii(9, standby_time);
  dec_to_ascii(11, use_EE_program);
  calculate_checksum(12);
  dec_to_ascii(13, checksum);
	Tx_buffer[15] = ']';
  command_transmitted = 0;
	Tx_index = 1;
  UDR0 = '[';                                     // start transmission
} /* t r a n s m i t _ s e t t i n g s */



void transmit_meter_counter(uint32_t *metercnt)
{
	uint8_t *temp;

	Tx_buffer[0] = '[';
	Tx_buffer[1] = '0';                            // command number = 05
	Tx_buffer[2] = '5';

  temp = (uint8_t *)(void *)metercnt;
  Tx_data = *temp;
  dec_to_ascii(3, Tx_data);

  temp++;
  Tx_data = *temp;
  dec_to_ascii(5, Tx_data);

  temp++;
  Tx_data = *temp;
  dec_to_ascii(7, Tx_data);

  calculate_checksum(8);
  dec_to_ascii(9, checksum);
	Tx_buffer[11] = ']';
  command_transmitted = 0;
	Tx_index = 1;
  UDR0 = '[';                                     // start transmission

} /* t r a n s m i t _ m e t e r _ c o u n t e r */



void transmit_error_counters(void)
{
	uint8_t temp;

	Tx_buffer[0] = '[';
	Tx_buffer[1] = '0';                            // command number = 07
	Tx_buffer[2] = '7';
  temp = read_EE_byte(EE_COVER_ERRORS);
  dec_to_ascii(3, temp);
  temp = read_EE_byte(EE_JAM_ERRORS);
  dec_to_ascii(5, temp);
  temp = read_EE_byte(EE_T_HIGH_ERRORS);
  dec_to_ascii(7, temp);
  temp= read_EE_byte(EE_T_LOW_ERRORS);
  dec_to_ascii(9, temp);
  temp = read_EE_byte(EE_DELTA_T_ERRORS);
  dec_to_ascii(11, temp);
  calculate_checksum(12);
  dec_to_ascii(13, checksum);
	Tx_buffer[15] = ']';
  command_transmitted = 0;
	Tx_index = 1;
  UDR0 = '[';                                     // start transmission
} /* t r a n s m i t _ e r r o r _ c o u n t e r s */



void transmit_TAG_info(void)
{
	Tx_buffer[0] = '[';
	Tx_buffer[1] = '0';                            // command number = 09
	Tx_buffer[2] = '9';
  dec_to_ascii(3, seal_temp_on_tag);
  dec_to_ascii(5, blower_speed_on_tag);
  dec_to_ascii(7, seal_position_on_tag);
  dec_to_ascii(9, driver_delay_on_tag);
  dec_to_ascii(11, foil_lenght_L);
  dec_to_ascii(13, foil_lenght_H);
  dec_to_ascii(15, roll_empty);
  dec_to_ascii(17, tag_state_for_comm);
  dec_to_ascii(19, foil_type);
  dec_to_ascii(21, foil_prod_year);
  dec_to_ascii(23, foil_prod_month);
  dec_to_ascii(25, foil_prod_day);
  dec_to_ascii(27, foil_prod_operator);
  calculate_checksum(28);
  dec_to_ascii(29, checksum);
	Tx_buffer[31] = ']';
  command_transmitted = 0;
	Tx_index = 1;
  UDR0 = '[';                                     // start transmission
} /* t r a n s m i t _ T A G _ i n f o */



void transmit_PID_parameters (void)
{
	Tx_buffer[0] = '[';
	Tx_buffer[1] = '6';                            // command number = 99
	Tx_buffer[2] = '3';
  dec_to_ascii(3, temperature);
  dec_to_ascii(5, average_T_heater/10);
  dec_to_ascii(7, setpoint);
  dec_to_ascii(9, P_heater);
  dec_to_ascii(11, I_heater);
  dec_to_ascii(13, D_heater);
  dec_to_ascii(15, on_time_heater);
  calculate_checksum(16);
  dec_to_ascii(17, checksum);
	Tx_buffer[19] = ']';
  command_transmitted = 0;
	Tx_index = 1;
  UDR0 = '[';                                     // start transmission
}   /* t r a n s m i t _ P I D _ p a r a m e t e r s */



void RS232_receive (void)
{
	uint8_t temp;

  if (command_received)
  {
  	check_checksum();
  	if (checksum_OK)
  	{
  		Rx_command = ascii_to_dec(1);

  		if (Rx_command == 1)                                         // Read a seal program
  			transmit_seal_program();


  		if (Rx_command == 2)                                         // Change a seal program
  		{
			  Rx_data = ascii_to_dec(3);
    	  write_EE_byte(EE_SEAL_TEMP, Rx_data);

			  Rx_data = ascii_to_dec(5);
    	  write_EE_byte(EE_BLOWER_SPEED, Rx_data);

			  Rx_data = ascii_to_dec(7);
    	  write_EE_byte(EE_ASSEMBLY_STEPS, Rx_data);

			  Rx_data = ascii_to_dec(9);
	  	  write_EE_byte(EE_DRIVER_DELAY, Rx_data);
  		}


  		if (Rx_command == 3)                                         // Read settings
  		{
  			transmit_settings();
   		}


  		if (Rx_command == 4)                                         // Change settings
  		{
			  Rx_data = ascii_to_dec(3);
			  if (Rx_data < 15)
			    Rx_data = 15;
			  if (Rx_data > 120)
			    Rx_data = 120;
			  temp = Rx_data / 5;
			  Rx_data = temp * 5;
    	  write_EE_byte(EE_STANDBY_TIME, Rx_data);
    	  standby_time = Rx_data;
  		}


  		if (Rx_command == 5)                                         // Read overall meter counter
  		{
        read_metercounter();
  			transmit_meter_counter(&metercount);
  		}

  		if (Rx_command == 6)                                         // Clear overall meter counter 
  		{
  			clear_metercounters();
  		}

  		if (Rx_command == 7)                                         // transmit error counters 
  		{
  			transmit_error_counters();
  		}

  		if (Rx_command == 8)                                         // clear error counters 
  		{
      	write_EE_byte(EE_COVER_ERRORS, 0x00);
      	write_EE_byte(EE_JAM_ERRORS, 0x00);
      	write_EE_byte(EE_T_HIGH_ERRORS, 0x00);
      	write_EE_byte(EE_T_LOW_ERRORS, 0x00);
      	write_EE_byte(EE_DELTA_T_ERRORS, 0x00);
  		}

  		if (Rx_command == 9)                                         // transmit TAG info
  		{
  			transmit_TAG_info();
  		}

  		if (Rx_command == 66)                                        // program TAG info
  		{
  			enable_tag_reading = 0;
  			tag_prg_step = 1;

			  tag_prg_seal_temp = ascii_to_dec(3);
        tag_prg_blower_speed = ascii_to_dec(5);
        tag_prg_seal_position = ascii_to_dec(7);
        tag_prg_driver_delay = ascii_to_dec(9);
        tag_prg_foil_lenght_L = ascii_to_dec(11);
        tag_prg_foil_lenght_H = ascii_to_dec(13);
        tag_prg_roll_empty = ascii_to_dec(15);
        tag_prg_foil_type = ascii_to_dec(17);
        tag_prg_foil_prod_year = ascii_to_dec(19);
        tag_prg_foil_prod_month = ascii_to_dec(21);
        tag_prg_foil_prod_day = ascii_to_dec(23);
        tag_prg_foil_prod_operator = ascii_to_dec(25);
  		}

  		if (Rx_command == 99)                                         // Read faults history
  		{
  			transmit_PID_parameters();
  		}

  		
  	}
  	command_received = 0;
  }
} /* R S 2 3 2 _ r e v e i c e */



