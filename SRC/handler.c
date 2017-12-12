#include "handler.h"
#include "system_timer.h"
#include "max.h"
#include "uart.h"
#include "led.h"

const u8 DIP_POLL_MODE=0x01;
const u8 NUMBER_LIMIT=0xFF;


//eai eiinoaioa yoi ia eiinoaioa I_i
#define STATE_NUMBER 0
#define STATE_CR 1
#define STATE_ERROR 2
#define NEWLINE 0x0A

u8 mode;
u8 number;
u8 state;

u8 read_dip(){
	return read_max(EXT_LO);
}

void reset() {
	number=0;
	state=STATE_NUMBER;
}

void initialize_handler() {
	mode = MODE_POLL;
	reset();
}

void doTriple() {
	u8 i;
	unsigned char bufChar;
	leds(0);
	byte_in = readDirect();
	while (read_dip()==DIP_POLL_MODE)
	{
		bufChar = readDirect();
		for( i=1;i<=3;i++ ){
			if (bufChar > 96 && bufChar < 123) {
				bufChar = bufChar - 32;
			}
			if (bufChar > 191 && bufChar < 224) {
				bufChar = bufChar + 32;
			}
			writeDirect(bufChar);
		}
		writeDirect(NEWLINE);
		delay_ms(1);
	}
	// while( read_dip()==DIP_POLL_MODE ){
	// 	
	// 	if( read_byte(&byte_in) ){
	// 		for( i=1;i<=3;i++ ){
	// 			if (byte_in > 96 && byte_in < 123) {
	// 				byte_in = byte_in - 32;
	// 			}
	// 			if (byte_in > 191 && byte_in < 224) {
	// 				byte_in = byte_in + 32;
	// 			}
	// 			send_byte(byte_in);
	// 		}
	// 		send_string("\r\n");
	// 	}
		
	// 	delay_ms(1);
	// }
	
	mode=MODE_COVERTER;
}

void doConvert() {
	while( read_dip()!=DIP_POLL_MODE ){
		delay_ms(1);
	}
	
	mode=MODE_POLL;
}

void modes_handle() {
	while(1) {
		if( mode==MODE_POLL ) {
			send_string("\r\nTripple mode (poll)\r\n");
			doTriple();
		}else{
			send_string("\r\nConverter (interuption)\r\n");
			doConvert();
		}
	}
}

void write_error() {
	send_string("\r\n Error happened\n");
	state=STATE_ERROR;
}


void convert() {
	u8 num;
	u8 sym;
	int array[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i;
	int count=0;
	
	if( state==STATE_ERROR ){//i?euaai iinea ioeaee
		reset();
	}
	
	if( read_byte(&sym) ){
		switch (state) {
			case STATE_NUMBER:
				if(sym>='0' && sym<='9'){
					send_byte(sym);
					num=sym-'0';
				
					if( num > NUMBER_LIMIT-number*10 ) {
						write_error();
						return;
					}
					
					number*=10;
					number+=num;
				}else if (sym == '\r'){
					send_string("\r\nBin:");
					while (number > 0) {
						array[count] = number % 2;
						number = number / 2;
						count++;
					}
					for (i = 7; i >= 0; i--) {
						send_byte(array[i] + '0');
						led(i, array[i]);
					}
					
					send_string("\r\n");
					reset();
				}
				else{
					write_error();
				}
				break;
		}
	} else 
		write_error();
}