/*
 * pwm_and_IR_with_Interrupts.c
 *
 * Created: 23-4-2013 18:14:15
 *  Author: Martijn
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "defines.h"
#include "init.c"
#include "isr.c"


int Get_RC5(void)
{
	TCNT2 = 0xE9;						// reset timer
			
	switch(bit_nr)
	{
		case 0:
			RC5_cmd = 0X00;
			bit_cnt = 0;
			while(IR_STATUS);
						
			if(bit_cnt == 20)
				bit_nr++;		
		break;
				
		case 1:
			bit_cnt = 0;
			while(IR_STATUS);
			
			if(bit_cnt == 20)
				bit_nr++;	
		break;
		
		case 2:
			bit_cnt = 0;
			while(IR_STATUS);
			
			if(bit_cnt == 20)
				bit_nr++;
		break;
		
		case 3:
			bit_cnt = 0;
			while(IR_STATUS);
			
			if(bit_cnt == 20)
				bit_nr++;
		break;	
		
		case 4:
			bit_cnt = 0;
			while(IR_STATUS);
			
			if(bit_cnt == 20)
				bit_nr++;		
		break;
		
		case 5:
			bit_cnt = 0;
			while(IR_STATUS);
						
			if(bit_cnt == 10)	
				RC5_cmd &= ~0x80;
			else
				RC5_cmd |= 0x80;	

			if(bit_cnt == 20)
				bit_nr++;				
		break;
		
		case 6:
			bit_cnt = 0;
			while(IR_STATUS);
			
			if(bit_cnt == 10)
				RC5_cmd &= ~0x40;		
			else
				RC5_cmd |= 0x40;	
			
			if(bit_cnt == 20)
				bit_nr++;		
		break;
		
		case 7:
			bit_cnt = 0;
			while(IR_STATUS);
						
			if(bit_cnt == 10)
				RC5_cmd &= ~0x20;
			else
				RC5_cmd |= 0x20;

			if(bit_cnt == 20)
				bit_nr++;			
		break;
		
		case 8:
			bit_cnt = 0;
			while(IR_STATUS);
						
			if(bit_cnt == 10)
				RC5_cmd &= ~0x10;
			else
				RC5_cmd |= 0x10;	

			if(bit_cnt == 20)
				bit_nr++;
		break;
		
		case 9:
			bit_cnt = 0;
			while(IR_STATUS);
						
			if(bit_cnt == 10)
				RC5_cmd &= ~0x08;
			else
				RC5_cmd |= 0x08;

			if(bit_cnt == 20)
				bit_nr++;				
		break;
		
		case 10:
			bit_cnt = 0;
			while(IR_STATUS);
						
			if(bit_cnt == 10)
				RC5_cmd &= ~0x04;
			else
				RC5_cmd |= 0x04;	

			if(bit_cnt == 20)			
				bit_nr++;			
		break;
		
		case 11:
			bit_cnt = 0;
			while(IR_STATUS);
			
			if(bit_cnt == 10)						
				RC5_cmd &= ~0x02;
			else
				RC5_cmd |= 0x02;

			if(bit_cnt == 20)
				bit_nr++;
		break;
		
		case 12:
			bit_cnt = 0;
			while(IR_STATUS);

			if(bit_cnt == 10)			
				RC5_cmd &= ~0x01;
			else	
				RC5_cmd |= 0x01;

			if(bit_cnt == 20)
				bit_nr = 0;
		break;		
	}		
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
	
			RC5_cmd = 0x00;
			Tx_buffer[7] = RC5_cmd;
			Tx_buffer[8] = bit_nr;
			Tx_buffer[9] = 0xAA;
			Tx_buffer[10] = test;
			Tx_buffer[11] = test2;						
			Tx_buffer[12] = test3;			
			Tx_buffer[13] = ETX;
			
			received = 0;
			Tx_index = 1;			
			// Verzend de Transmit Buffer
			UDR0 = STX;  // start transmission									
		}	
	}	
}