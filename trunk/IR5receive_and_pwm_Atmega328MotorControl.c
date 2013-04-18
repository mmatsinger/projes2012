/*
 * IR5receive_and_pwm_Atmega328MotorControl.c
 *
 * Created: 16-4-2013 18:54:38
 *  Author: Martijn
 */ 

/*
 * pwmAtmega328MotorControl.c
 *
 * Created: 5-1-2013 19:44:19
 *  Author: George Okeke
 */ 

#define F_CPU 		   8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "ircodes.h"

#define IRpin_PIN      PIND
#define IRpin          4

// the maximum pulse we'll listen for - 65 milliseconds is a long time
#define MAXPULSE 65000
#define NUMPULSES 50
#define RESOLUTION 20 

// What percent we will allow in variation to match the same code
#define FUZZINESS 20

// we will store up to 100 pulse pairs (this is -a lot-)
uint16_t pulses[NUMPULSES][2];  // pair is high and low pulse 
uint8_t currentpulse = 0; // index for pulses we're storing
uint8_t RC5_cmd = 0;

void setup(void) 
{
//  Serial.begin(9600);
//  Serial.println("Ready to decode IR!");
}

void loop(void) 
{
  int numberpulses;
  
  numberpulses = listenForIR();
  
//  Serial.print("Heard ");
//  Serial.print(numberpulses);
//  Serial.println("-pulse long IR signal");
  if (IRcompare(numberpulses, ApplePlaySignal,sizeof(ApplePlaySignal)/4)) 
  {
//    Serial.println("PLAY");
  }
    if (IRcompare(numberpulses, AppleRewindSignal,sizeof(AppleRewindSignal)/4)) 
	{
//    Serial.println("REWIND");
    }
    if (IRcompare(numberpulses, AppleForwardSignal,sizeof(AppleForwardSignal)/4)) 
	{
//     Serial.println("FORWARD");
    }
  _delay_ms(500);
}

//KGO: added size of compare sample. Only compare the minimum of the two
int IRcompare(int numpulses, int Signal[], int refsize) 
{
  int count = 0;
  if(numpulses < refsize)
  {
	  count = numpulses;
  }
  else
  {
	count = refsize;
  }	  
//  int count = min(numpulses,refsize);
//  Serial.print("count set to: ");
//  Serial.println(count);
  for (int i=0; i< count-1; i++) 
  {
    int oncode = pulses[i][1] * RESOLUTION / 10;
    int offcode = pulses[i+1][0] * RESOLUTION / 10;
    
#ifdef DEBUG    
//    Serial.print(oncode); // the ON signal we heard
//    Serial.print(" - ");
//    Serial.print(Signal[i*2 + 0]); // the ON signal we want 
#endif   
    
    // check to make sure the error is less than FUZZINESS percent
    if ( abs(oncode - Signal[i*2 + 0]) <= (Signal[i*2 + 0] * FUZZINESS / 100)) 
	{
		#ifdef DEBUG
//      Serial.print(" (ok)");
		#endif
    } 
	else 
	{
		#ifdef DEBUG
//      Serial.print(" (x)");
		#endif
      // we didn't match perfectly, return a false match
		return 0;
    }
    
    
#ifdef DEBUG
//    Serial.print("  \t"); // tab
//    Serial.print(offcode); // the OFF signal we heard
//    Serial.print(" - ");
//    Serial.print(Signal[i*2 + 1]); // the OFF signal we want 
#endif    
    
    if ( abs(offcode - Signal[i*2 + 1]) <= (Signal[i*2 + 1] * FUZZINESS / 100)) 
	{
		#ifdef DEBUG
//		Serial.print(" (ok)");
		#endif
    } 
	else 
	{
		#ifdef DEBUG
//      Serial.print(" (x)");
		#endif
      // we didn't match perfectly, return a false match
      return 0;
    }
    
	#ifdef DEBUG
//		Serial.println();
	#endif
  }
  // Everything matched!
  return 1;
}

int listenForIR(void) 
{
  currentpulse = 0;
  
  while (1) 
  {
    uint16_t highpulse, lowpulse;  // temporary storage timing
    highpulse = lowpulse = 0; // start out with no pulse length
  
    while (IRpin_PIN & (1 << IRpin)) 
	{
       // pin is still HIGH

       // count off another few microseconds
       highpulse++;
       _delay_ms(RESOLUTION);

       // If the pulse is too long, we 'timed out' - either nothing
       // was received or the code is finished, so print what
       // we've grabbed so far, and then reset
       
       // KGO: Added check for end of receive buffer
       if (((highpulse >= MAXPULSE) && (currentpulse != 0))|| currentpulse == NUMPULSES) 
	   {
         return currentpulse;
       }
    }
    // we didn't time out so lets stash the reading
    pulses[currentpulse][0] = highpulse;
  
    // same as above
    while (! (IRpin_PIN & _BV(IRpin))) 
	{
       // pin is still LOW
       lowpulse++;
       _delay_ms(RESOLUTION);
        // KGO: Added check for end of receive buffer
        if (((lowpulse >= MAXPULSE)  && (currentpulse != 0))|| currentpulse == NUMPULSES) 
		{
         return currentpulse;
       }
    }
    pulses[currentpulse][1] = lowpulse;

    // we read one high-low pulse successfully, continue!
    currentpulse++;
  }
}

void printpulses(void) {
/*
  Serial.println("\n\r\n\rReceived: \n\rOFF \tON");
  for (uint8_t i = 0; i < currentpulse; i++) {
    Serial.print(pulses[i][0] * RESOLUTION, DEC);
    Serial.print(" usec, ");
    Serial.print(pulses[i][1] * RESOLUTION, DEC);
    Serial.println(" usec");
  }
  
  // print it in a 'array' format
  Serial.println("int IRsignal[] = {");
  Serial.println("// ON, OFF (in 10's of microseconds)");
  for (uint8_t i = 0; i < currentpulse-1; i++) {
    Serial.print("\t"); // tab
    Serial.print(pulses[i][1] * RESOLUTION / 10, DEC);
    Serial.print(", ");
    Serial.print(pulses[i+1][0] * RESOLUTION / 10, DEC);
    Serial.println(",");
  }
  Serial.print("\t"); // tab
  Serial.print(pulses[currentpulse-1][1] * RESOLUTION / 10, DEC);
  Serial.print(", 0};");
*/  
}


// initialize the usart
void USART_Init(void)
{
	UBRR0H = 0x00;		// 9600 bps (@ 8MHz)
	UBRR0L = 0x33;
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);		// receive and transmit Enable
	UCSR0C |= 0x06;		//8 databit, geen pariteit, 1 stopbit
	UCSR0A = (UCSR0A & 0xFD);
}

// This program sends karakter
void USART_Transmit (unsigned char data)	// pollend zenden van een karakter
{
	while (!(UCSR0A & 0x20)) 
	{		// wacht totdat zendbuffer(UDRE) leeg is
	}

	UDR0 = data;			// zend data
	UCSR0A = (UCSR0A | (1<<TXC0));
}

// This program receives karakter
unsigned char USART_Receive (void)			// pollend ontvangen van karakter
{
	unsigned char data;
	
	while (!(UCSR0A & 0x80)) 
	{	//wacht totdat karakter binnen is (RXC bit set)
		
	}		
	
	data = UDR0;
	
	return data;			// geef karakter terug aan programma
}

// this program flushes the receive and transmit registers
void USART_Flush( void )
{
	unsigned char trowaway;
	
	while ( UCSR0A & (1<<RXC0) ) 
	{
		trowaway = UDR0;
	}
}


int main(void)
{
	// Initialize the USART
	USART_Init();
	
	// Enable the PRTIM1 bit in Power Reduction Register to disable the Timer/Counter1
	//PRR = (PRR | (1 << PRTIM1) | (1 << PRTIM0) | (1 << PRUSART0));
	
	// (WGM13:0) is set to mode 9(PWM, Phase and Frequency Correct, 16-bit)
	// (COM1B1:0) is set to mode 3(set OC1B on compare match)
	// (CS12:0) is set to mode 1 (clk/1)from prescaler

	TCCR1A = 0xF0;	//bit7: compare output mode for compare unit A(COM1A1)
					//bit6: compare output mode for compare unit A(COM1A0)
					//bit5: compare output mode for compare unit B(COM1B1)
					//bit4: compare output mode for compare unit B(COM1B0)
					//bit3: 
					//bit2: 
					//bit1:0 Waveform Generation Mode (WGM11:0)
	TCCR0A = 0xA3;	//bit7: compare output mode for compare unit A(COM1A1)
					//bit6: compare output mode for compare unit A(COM1A0)
					//bit5: compare output mode for compare unit B(COM1B1)
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

	DDRB = 0xFF;	// PORTB the direction is defined as OUTPUT
	DDRD = 0xEF;	// PortD is defined as OUTPUT - P4D is INPUT (RC5 receive)
	
	// defines the top values used in generating pwm for the servo motor
	ICR1H = 0xFF;
	ICR1L = 0xFF;
	
	// after setting the counter registers value then disable the PRTIM1 bit in Power Reduction Register to enable the Timer/Counter1
	//  and disable the PRTIM0 bit in the Power Reduction register to enable the Timer/counter0
	// and disable the Power Reduction USART bit (PRUART0) in order to make use of the usart.
//	PRR = (PRR & 0xD5);
	
	unsigned int count = 0;
	const unsigned char BUFLEN = 7;
	unsigned char PACKETBEGIN = 1;
	unsigned char recvBuf[] = {0, 0, 0, 0, 0, 0, 0 };    // byte 0: check byte must be 1.
														// byte 1: OCR1AH value is the High byte for the left servo motor
														// byte 2: OCR1AL value is the Low byte for the left servo motor
														// byte 3: OCR1BH value is the High byte for the right servo motor
														// byte 4: OCR1BL vlaue is the Low byte for the left servo motor
														// byte 5: OCR0B  value is the speed control for the DC motor	
														// byte 6: RC5 command to send back
	
	
	// only used as the default values
	OCR1AH = 0x00;
	OCR1AL = 0x00;
	OCR1BH = 0x00;
	OCR1BL = 0x00;	
	OCR0B  = 0x00;
	
	PORTB = 0x00;
	//--------------------
	USART_Flush();
		
    while(1)
    {
		// receive the control data's (BLOCK if there is no data to receive)
		for (count = 0; count < BUFLEN; count++) 
		{
			recvBuf[count] = USART_Receive();
		}

		RC5_cmd = 0x09; 		// FOR TESTING !!!;		
		recvBuf[6] = RC5_cmd;

		// send back the received control data's for correctness checking
		for (count = 0; count < BUFLEN; count++) 
		{
			USART_Transmit(recvBuf[count]);
		}

		if (recvBuf[0] == PACKETBEGIN) {
			// update the registers with the new values
			OCR1AH = recvBuf[1];
			OCR1AL= recvBuf[2];
			OCR1BH = recvBuf[3];
			OCR1BL= recvBuf[4];
			OCR0B = recvBuf[5];
		}		
		loop();
	}		
}

