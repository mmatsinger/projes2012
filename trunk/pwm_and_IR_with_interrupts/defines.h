#define nop()  __asm__ __volatile__ ("nop" ::)
#define F_CPU 	    8000000UL

#define ADDR_BEAGLE	0xA1
#define ADDR_PC		0xA2
#define STX			0x02
#define ETX			0x03

#define IR_INPUT    PD2									// the IR-Decoder has to be connected to an interrupt pin
#define IR_STATUS	(!IR_INPUT)							// invert signal from IR-Receiver

#define BUFLEN		9
unsigned int received = 0;
unsigned int count = 0;
unsigned int start = 1;

unsigned int test = 0;
unsigned int test2 = 0;
unsigned int test3 = 0;
unsigned int bit_nr = 1;
unsigned int bit_cnt = 0;
unsigned char FROMBEAGLE = 0x01;

uint8_t RC5_cmd = 0;				// the received RC5 instruction


struct rc5_struct
{
	int8_t data[2];
	int8_t state;
};

uint8_t
cnt,
Rx_index,
Tx_index,
command_received,
command_transmitted,

Rx_data,
Tx_data,
Rx_command,
last_Tx_byte,

Rx_buffer[35],
Tx_buffer[35];

struct rc5_struct RC5;
