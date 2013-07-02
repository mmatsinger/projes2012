/*
 * pwm_and_IR_with_Interrupts.c
 *
 * Created: 23-4-2013 18:14:15
 *  Author: Martijn
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 	    8000000UL

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

int8_t get_RC5(void);

#include "defines.h"
#include "init.c"
#include "isr.c"

// LINK = www.martinhubacek.cz/atmel/ir-remote-control-decoder

void rc5_decode(void)
{
	if(bit_nr < 12)
	{
		bit_cnt = 0;		
		while(IR_LOW)
		{
			PORTD |= (1 << PD4);
		}

		bit_time_low[bit_nr] = bit_cnt;
		
		bit_cnt = 0;
		while(IR_HIGH)
		{
			PORTD &= ~(1 << PD4);			
		}
		bit_time_high[bit_nr] = bit_cnt;
		
		bit_nr++;
	}
	
	if(bit_nr == 11)
		bit_nr = 0;
}	

/*
#include <16F648A.h>
#use delay(clock=20000000)
#fuses NOWDT,HS, PUT, NOPROTECT, BROWNOUT, MCLR, NOLVP, NOCPD

void get_RC5(void);


#define IR_INPUT PIN_B0         // the IR-Decoder has to be connected to an interrupt pin

#define IR_STATUS (!input(IR_INPUT))  // invert signal from IR-Receiver

typedef struct
{
	int8 data[2];
	int8 state;
} rc5_struct;

rc5_struct rc5;

// interrupt function for external interrupt pin
#int_EXT
EXT_isr()
{
	get_RC5();
}



// RC5 decoding
int1 get_RC5(void)
{
	int16 tmp,t;
	int i;
	int1 inp;

	set_timer1(0);                  // reset timer
	while(IR_STATUS==1);
	t=get_timer1();

	if ((t<400) || (t<800)) return 0;   // no RC5 code, cancel decoding

	// receive all 13 data bits
	for (i=0;i<13;i++)
	{

		inp=IR_STATUS;
		set_timer1(0);

		while (IR_STATUS==inp)
		{
			t=get_timer1();
			if (t>800) return 0;         // timing wrong -> no RC5
			
		}

		tmp<<=1;

		if (inp==0) tmp++;

		set_timer1(0);
		while (get_timer1()<776);
	}

	tmp=tmp | 0x3000;      // clear Bits 13,14 & 15

	tmp=tmp & 0x37ff;      // cut off Togglebit

	rc5.data[0]=tmp & 0xff;  // extract device address

	tmp>>=8;

	rc5.data[1]=tmp & 0xff;  // extract command code

	rc5.state = 1;         // set flag for successful IR command decoding

	disable_interrupts(INT_EXT);      // disable Interrupts until received command has been processed

	return 1;
}




void main()
{
	// controller initialisieren
	setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
	setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);      // timer1 is used for RC5-decoding. When the chip runs @ 20MHz,
	// a prescaler of 8 has to be used
	
	setup_timer_2(T2_DISABLED,0,1);
	setup_comparator(NC_NC_NC_NC);
	setup_vref(FALSE);

	// enable interrupts
	enable_interrupts(INT_EXT);
	enable_interrupts(GLOBAL);

	rc5.state = 0;

	while(1)
	{
		if(rc5.state==1)  // did we receive a valid command?
		{

			rc5.state = 0;

			printf("Device: %u\n", rc5.data[0]);   // display device address
			printf("Command: %u\n", rc5.data[1]);    // display command code

			enable_interrupts(INT_EXT);   // reenable interrupts
		}

	}


}



*/			

int8_t get_RC5(void)
{
	int16_t tmp;
	int16_t d_t;
	int8_t i;
	int8_t inp;

	TCNT2 = 0;                  // reset timer
	while(IR_STATUS == 1);
	d_t = TCNT2;

	if ((d_t < 80) || (d_t < 160)) return 0;   // no RC5 code, cancel decoding

	// receive all 13 data bits
	for (i = 0; i< 13; i++)
	{
		inp = IR_STATUS;
		TCNT2 = 0;
		
		while (IR_STATUS == inp)
		{
			d_t = TCNT2;
			if (d_t > 800) return 0;         // timing wrong -> no RC5			
		}

		tmp <<= 1;

		if (inp == 0) tmp++;

		TCNT2 = 0;
		while (TCNT2 <776);
	}

	tmp = tmp | 0x3000;      // clear Bits 13,14 & 15

	tmp = tmp & 0x37ff;      // cut off Togglebit

	RC5.data[0] = tmp & 0xff;  // extract device address

	tmp >>= 8;

	RC5.data[1] = tmp & 0xff;  // extract command code

	RC5.state = 1;         // set flag for successful IR command decoding

	TIMSK2 &= ~(1 << TOIE2);      // disable Interrupts until received command has been processed
	EIMSK &= ~(1 << INT0);		  // disable External interrupt
	return 1;
}

int main(void)
{
    Init();
		
	while(1)
    {										
		if(start)
		{
			PORTD |= (1 << PD3);	
			start = 0;
		}
		
		if(RC5.state == 1)
		{
			RC5.state = 0;
			EIMSK |= (1 << INT0);	
		}
		
		if(received)
		{	
			Rx_index = 1;
														
			if(Rx_buffer[1] == FROMBEAGLE)
			{				
				// update the registers with the new values
				OCR1AH = Rx_buffer[2];
				OCR1AL = Rx_buffer[3];
				OCR1BH = Rx_buffer[4];
				OCR1BL = Rx_buffer[5];
				OCR0B  = Rx_buffer[6];
			}
						
			// Vul de Zend Buffer met ontvangen data
			Tx_buffer[0] = STX;
			Tx_buffer[1] = FROMBEAGLE;
			Tx_buffer[2] = Rx_buffer[2];
			Tx_buffer[3] = Rx_buffer[3];
			Tx_buffer[4] = Rx_buffer[4];
			Tx_buffer[5] = Rx_buffer[5];
			Tx_buffer[6] = Rx_buffer[6];
	
//			RC5_cmd = 0x10;
			Tx_buffer[7] = RC5_cmd;
			Tx_buffer[8] = ETX;
			
			received = 0;
			Tx_index = 1;			
			// Verzend de Transmit Buffer
			UDR0 = STX;  // start transmission									
		}	
	}	
}