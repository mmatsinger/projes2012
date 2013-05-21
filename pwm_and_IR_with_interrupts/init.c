
void Init(void)
{
  DDRB = 0xFF;	// PORTB the direction is defined as OUTPUT
  DDRD = 0xFA;	// PortD is defined as OUTPUT - P2D is INPUT (RC5 receive, via interrupt ) (PD0 = input = RxD UART)
  PORTD &= ~(1 << PD2);
    
  /* INT0 registers */
  EICRA = 0x02;              // set IRQ vlag on falling edge of INT0 (PD2 pin32).
  EIFR = 0x00;               // clear INT0 vlag
  EIMSK = 0x01;              // enable INT0 interrupts

  /* Timer0 registers */  
  TCCR0A = 0x00;             // clear OCA0 and OCB0 on compare match.  Set on top.  Fast PWM
  TCCR0B = 0x04;             // prescaler 1:256
  TCNT0 = 0x00;              // timer0 = 63h   10msec = 40000 * 0,25usec --> 40000/1024 = 156 --> 255-156=63h
  OCR0A = 0x00;              // PWM_A = 0
  OCR0B = 0x00;              // PWM_B = 0
  
  TIFR2 = 0x00;              // clear Timer 2 vlag
  TIMSK2 = 0x01;             // enable Timer 2 overflow interrupts

  /* Timer1 registers */
  TCCR1A = 0x01;             // clear OCA0 and OCB0 on compare match.  Set on top.  Fast PWM 10-bit
  TCCR1C = 0x00;             // No prescaler
  TCNT1 = 0;                 // timer1 = 0
  OCR1A = 0;                 // PWM A = 0
  OCR1B = 0;                 // PWM B = 0
  TIFR1 = 0x00;              // clear Timer 1 vlag
  TIMSK1 = 0x00;             // disable all Timer 1 interrupts
  
  TCCR2A = 0X00;
  TCCR2B = 0X03;			// prescaler op 32  888 us = 7111 * 0,125us --> 7111 / 32 = 222 --> 255-222 = 0x21(Hex)
  TIMSK2 = 0x01;	
			
  TCCR1A = 0xF0;	//bit7: compare output mode for compare unit A(COM1A1)
  //bit6: compare output mode for compare unit A(COM1A0)
  //bit5: compare output mode for compare unit B(COM1B1)
  //bit4: compare output mode for compare unit B(COM1B0)
  //bit3:
  //bit2:
  //bit1:0 Waveform Generation Mode (WGM11:0)
  TCCR0A = 0xA3;	//bit7: compare output mode for compare unit A(COM1A1)
  //bit6: compare output mode for compare unit A(COM1A0)
  //bit5: compare output mode for com\pare unit B(COM1B1)
  //bit4: compare output mode for compare unit B(COM1B0)
  //bit3:
  //bit2:
  //bit1:0 Waveform Generation Mode (WGM11:0)
	
  TCCR1B = 0x11;	//bit7: Input Capture Noise Canceler(ICNC1
  //bit6: Input Capture Edge Select(ICES1)
  //bit5: - Reserved bit
  //bit4:3 Waveform Generation Mode(WGM13:2)
  //bit2:0 Clock Select(CS12:0)
  TCCR0B = 0x01;	//bit4:3 Waveform Generation Mode(WGM13:2)
  //bit2:0 Clock Select(CS12:0)

  // defines the top values used in generating pwm for the servo motor
  ICR1H = 0xFF;
  ICR1L = 0xFF;
		
  // only used as the default values
  OCR1AH = 0x00;
  OCR1AL = 0x00;
  OCR1BH = 0x00;
  OCR1BL = 0x00;
  OCR0B  = 0x00;
  
  PORTB = 0x00;

  /* USART 0 */
  UBRR0H = 0x00;						// 9600 bps (@ 8MHz)
  UBRR0L = 0x33;
  UCSR0B |= (1<<RXEN0)|(1<<TXEN0);		// receive and transmit Enable
  UCSR0B |= (1 << RXCIE0); 				// USART Receive Complete interrupt
  UCSR0B |= (1 << TXCIE0);				// Transmit Interrupt Complete Enabled

  UCSR0C |= 0x06;						//8 databit, geen pariteit, 1 stopbit
    
  for(count = 0; count < 35; count++)
  {
	  Tx_buffer[count] = 0;
	  Rx_buffer[count] = 0;	  
  }
  
  Rx_index = 1;
  Tx_index = 0;
  last_Tx_byte = 0;
  received = 0;
  sei ();                    // enable interrupts
//  cli();					   // disable interrupts
}  /* i n i t */
