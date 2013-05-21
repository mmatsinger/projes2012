
ISR (TIMER2_OVF_vect)
{
  TIFR2 = 0x00;                   // clear Timer 2 vlag
  TCNT2 = 0xE9;					  // 888 us = 7111 * 0,125us --> 7111 / 32 = 222 --> 255-222 = 0x20(Hex)
  
  bit_cnt++;
  if(bit_cnt > 22)
	bit_nr = 0;
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
	test3++;
	Get_RC5();
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

