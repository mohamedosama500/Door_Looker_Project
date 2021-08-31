/*
 * mainprogram1.c
 *
 *  Created on: 14 Sep 2020
 *      Author: Hp
 */

#include "lcd.h"
#include "keypad.h"
#include "UART.h"
#define NULL (void*)0
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

			LCD_sendCommand(CLEAR_COMMAND);
			LCD_displayStringRowColumn(0, 0, " Rotation");
			LCD_displayStringRowColumn(1, 0, " Anti Clock Wise ");
			LCD_goToRowColumn(3, 0);
		}

		else if (tick2 == 30) {
			LCD_sendCommand(CLEAR_COMMAND);
			LCD_displayStringRowColumn(0, 0, "+ : Open Door");
			LCD_displayStringRowColumn(1, 0, "- : Change Pass");
			LCD_goToRowColumn(3, 0);
			tick2=0;
		}

	}
}
int main(void) {
	unsigned char n_pass = 0;
	unsigned char key[n_pass];
	unsigned char *ptr1 = key;
	//unsigned char count_wrong=0;
	unsigned char n = 0; /* it's a variable used to compare it with size of the array that contain
	 numbers of the password first entered */
	unsigned char *ptr2 = NULL;/* initialization  of the second pointer that will take the second enter of
	 the password to compare it with the first pointer that stored these numbers in an array  */
	UART_init();
	LCD_init();
	LCD_displayStringRowColumn(0, 0, "Please enter new");
	LCD_displayStringRowColumn(1, 0, "password :");
	LCD_goToRowColumn(3, 0);
	DDRD = DDRD & (~(1 << PD0));    // configure pin 3 of PORTC as output pin
	DDRD = DDRD | (1 << PD1);    // configure pin 4 of PORTC as output pin
	while (1) {
		*ptr1 = KeyPad_getPressedKey(); /* get the pressed key number */
		/*displaying the number that presses in the LCD by checking it as it must be from
		 0 to 9*/
		if ((*ptr1 <= 9) && (*ptr1 >= 0)) {
			LCD_intgerToString(*ptr1); /* display the pressed keypad switch */
			ptr1++;
			n_pass++;
			UART_sendByte('c');

		}
		/*check if the password entered is more than 4 numbers with enter button pressed*/

		else if (*ptr1 == 13 && n_pass >= 4) {
			LCD_sendCommand(CLEAR_COMMAND);
			LCD_displayStringRowColumn(0, 0, "Please re-enter");
			LCD_displayStringRowColumn(1, 0, " password :");
			LCD_goToRowColumn(3, 0);
			/*while loop that will get out when it checks from all of the elements of the array that contain
			 that password entered first*/
			while (n != n_pass) {
				*ptr2 = KeyPad_getPressedKey(); /* get the pressed key number */
				/*checking the number pressed is the number stored first in the array then
				 displaying it in the LCD*/
				if (*ptr2 == key[n]) {
					n++;
					LCD_intgerToString(*ptr2);
					if (n == n_pass) {
						LCD_sendCommand(CLEAR_COMMAND);
						LCD_displayStringRowColumn(0, 0, "+ : Open Door");
						LCD_displayStringRowColumn(1, 0, "- : Change Pass");
						LCD_goToRowColumn(3, 0);
					}
				}
				/*if one of the numbers of the password entered wrong, then the user will enter new
				 password and the variables whether the size of array or the checking variable or the
				 first pointer it self, they will initialized again*/
				else if (*ptr2 != key[n]) {
					LCD_sendCommand(CLEAR_COMMAND);
					LCD_displayStringRowColumn(0, 0, "Please enter new");
					LCD_displayStringRowColumn(1, 0, "password :");
					LCD_goToRowColumn(3, 0);
					n = 0;
					ptr1 = key;
					n_pass = 0;
					//count_wrong++;
					//if(count_wrong>=3){
						UART_sendByte('b');
					//}
					break;/*to exit from the loop*/
				}
				_delay_ms(500); /* Press time */
			}

		}
		/*sending a character command to the second microcontroller through uart to launch the DC motor*/
		else if (*ptr1 == '+') {
			UART_sendByte('a');
			LCD_sendCommand(CLEAR_COMMAND);
			LCD_displayStringRowColumn(0, 0, " Rotation");
			LCD_displayStringRowColumn(1, 0, " Clock Wise ");
			LCD_goToRowColumn(3, 0);
			SREG |= (1 << 7);  // Enable global interrupts in MC.
			Timer_init_Normal_Mode(); //start the timer.
		}
		/* it will enter new
		 password and the variables whether the size of array or the checking variable or the
		 first pointer it self, they will initialized again*/
		else if (*ptr1 == '-') {
			LCD_sendCommand(CLEAR_COMMAND);
			LCD_displayStringRowColumn(0, 0, "Please enter new");
			LCD_displayStringRowColumn(1, 0, "password :");
			LCD_goToRowColumn(3, 0);
			n_pass = 0;
			n = 0;
			ptr1 = key;
		}
		/*if one of the numbers of the password less than four numbers and then enter button is pressed,
		 then the user will enter new
		 password and the variables whether the size of array or the checking variable or the
		 first pointer it self, they will initialized again*/
		else if (n_pass < 4 && *ptr1 == 13) {
			LCD_sendCommand(CLEAR_COMMAND);
			LCD_displayStringRowColumn(0, 0, "Please enter new");
			LCD_displayStringRowColumn(1, 0, "password :");
			LCD_goToRowColumn(3, 0);
			n_pass = 0;
			n = 0;
			ptr1 = key;
		}
		/*to show the pressed number anyway in the LCD*/
		else {
			LCD_displayCharacter((*ptr1)); /* display the pressed keypad switch */
		}

		_delay_ms(500); /* Press time */
	}

}

