#define ADDR_BEAGLE	0xA1
#define ADDR_PC		0xA2
#define STX			0x02
#define ETX			0x03

#define IRpin_PIN      PIND
#define IRpin          2

#define MAXPULSE 65000
#define NUMPULSES 20
#define RESOLUTION 20

// What percent we will allow in variation to match the same code
#define FUZZINESS	20
#define BUFLEN		9
unsigned int count = 0;
unsigned char FROMBEAGLE = 0x01;

uint16_t pulses[NUMPULSES][2];  	// pair is high and low pulse
uint8_t currentpulse = 0; 			// index for pulses we're storing
uint8_t RC5_cmd = 0;				// the received RC5 instruction

uint8_t

byte_L,
byte_H,

t_10msec,                      // 10 msec timing for main program
t_100msec,                     // 100 msec timing for main program
t_250msec,                     // 250 msec timing for main program
t_500msec,                     // 500 msec timing for main program
t_1sec,                        // 1 sec timing for main program
cnt_100msec,
cnt_250msec,
cnt_500msec,
cnt_1sec,
sec60,
minute,
t_20_min,

cnt_5sec,
ctr,
sec_ctr,
min_ctr,
cnt,
Rx_index,
Tx_index,
command_received,
command_transmitted,

steps_count,
checksum_OK,
checksum,
Rx_data,
Tx_data,
Rx_command,
last_Tx_byte,

Rx_buffer[35],
Tx_buffer[35];