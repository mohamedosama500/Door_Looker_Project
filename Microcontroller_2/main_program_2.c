#include "micro_config.h"
#include "external_eeprom.h"
#include "UART.h"
#include <avr/interrupt.h>

#define NUMBER_OF_OVERFLOWS_PER_SECOND 15

unsigned char tick = 0;
unsigned char tick2 = 0;
/* with clock=1Mhz and prescale F_CPU/256 every count will take 256 us
 * so put initial timer counter=0  0 --> 255 (255 * 32us = 65.526 ms per overflow)
 * so we need timer to overflow 15 times to get a 1 second
 */

void Timer_init_Normal_Mode(void) {
	TCNT0 = 0; //timer initial value
	TIMSK = (1 << TOIE0); //enable overflow interrupt
	/* configure the timer
	 * 1. Non PWM mode FOC0=1
	 * 2. Normal Mode WGM01=0 & WGM00=0
	 * 3. Normal Mode COM00=0 & COM01=0
	 * 4. clock = F_CPU/256 CS00=0 CS01=0 CS02=1
	 */
	TCCR0 = (1 << FOC0) | (1 << CS02);
}

ISR(TIMER0_OVF_vect) {
	tick++;
	if (tick == NUMBER_OF_OVERFLOWS_PER_SECOND) {
		tick = 0;
		tick2++;
		if (tick2 == 15) {

			PORTB &= ~((1 << PB0) | (1 << PB1));
			PORTB = PORTB & (~(1 << PB0));
			PORTB = PORTB | (1 << PB1);
		}

		else if (tick2 == 30) {
			PORTB &= ~((1 << PB0) | (1 << PB1));

		}

	}
}

int main(void) {
	uint8 val2 = 0;
	DDRD = 0xFF;
	PORTD = 0x00;
	UART_init();

	DDRB = DDRB | (1 << PB0);    // configure pin 3 of PORTB as output pin
	DDRB = DDRB | (1 << PB1);    // configure pin 4 of PORTB as output pin
	DDRC = DDRC | (1 << PC3);    // configure pin 3 of PORTC as output pin
	DDRD = DDRD & (~(1 << PD0));    // configure pin 3 of PORTD as output pin
	DDRD = DDRD | (1 << PD1);    // configure pin 4 of PORTD as output pin
	PORTC = PORTC & (~(1 << PC3));
	while (1) {
		val2 = UART_recieveByte();
		if (val2 == 'a') {
			PORTB = PORTB | (1 << PB0);
			PORTB = PORTB & (~(1 << PB1));
			SREG |= (1 << 7);  // Enable global interrupts in MC.
			Timer_init_Normal_Mode(); //start the timer.

		}
		else if(val2 == 'b'){
			PORTC = PORTC | (1 << PC3);
		}
		else if(val2=='c'){
			PORTC = PORTC & (~(1 << PC3));
		}
	}
	return 0;
}

