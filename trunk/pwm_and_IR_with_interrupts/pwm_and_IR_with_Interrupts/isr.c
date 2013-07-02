
ISR (TIMER2_OVF_vect)
{
  TIFR2 = 0x00;                   // clear Timer 2 vlag
  TCNT2 = 0xD2;					  // 88 us = 711 * 0,125us --> 711 / 32 = 22 --> 255-22 = 0xE9(Hex)
  
  bit_cnt++;

/* 
  if(test2)
  {
	  PORTD |= (1 << PD4);
	  test2 = 0;
  }	
  else
  {
	  PORTD &= ~(1 <<PD4);
	  test2 = 1;
  }
*/       
}   /* Timer_2 overflow  ISR */



ISR (INT0_vect)
{
//	rc5_decode();
	if (test3)
	{
		PORTD &= ~(1 << PD4);
		test3 = 0;
	}
	else
	{
		PORTD |= (1 << PD4);
		test3 = 1;
	}		
	
	get_RC5();
}   /* c o n v e r s i o n _ r e a d y _ I S R */



ISR (USART_RX_vect)                // USART receive complete interrupt
{
	Rx_data = UDR0;
	if (Rx_data == STX)
	{
	  PORTD |= (1 << PD3);

	  Rx_index = 1;
	  Rx_buffer[Rx_index] = Rx_data;
	}	  
	else
	{
		if (Rx_data == ETX)
		{
		  PORTD &= ~(1 << PD3);
		  Rx_buffer[Rx_index] = Rx_data;
		  received = Rx_index;   // command_received points to the last position in Rx_buffer 		  
		}		  
		else
		{
			Rx_buffer[Rx_index] = Rx_data;
			if (Rx_index < 30)
				Rx_index++;
	  }
	}
}


ISR (USART_TX_vect)               // USART transmit complete interrupt
{
  if (last_Tx_byte)
  {
//	    UCSR0B &= 0x40;                    // disable transmit interrupt
	    last_Tx_byte = 0;
  }
  else
  {
    UDR0 = Tx_buffer[Tx_index];

	if ( (Tx_buffer[Tx_index] == ETX) || (Tx_index >= 35) )
  	{
	    last_Tx_byte = 1;
	    command_transmitted = 1;
	}
    
	if (Tx_index < 35)
	{
      Tx_index++;
	}	  
  }
}

