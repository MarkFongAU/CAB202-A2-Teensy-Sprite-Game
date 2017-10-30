/*	CAB202: Tutorial 10
*	Question 4 - Template
*
*	B.Talbot, May 2016
*	Queensland University of Technology
*/
#define __AVR_ATmega32U4__
#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "cpu_speed.h"
#include "graphics.h"
#include "lcd.h"

/*
* Defines for calculating our baud rate
*/
#define CPU_SPEED 1000000UL            // TODO Use the clock speed we compile against...
#define DESIRED_BAUD_RATE 9600         // TODO Use an appropriate value...
#define BAUD_PRESCALER ((CPU_SPEED)/(DESIRED_BAUD_RATE*16UL)-1)

/*
* Function declarations
*/
void init_hardware(void);
void UART_send_char(char data);

unsigned char debounced_right_press();

/*
* Main
*/
int main() {
	set_clock_speed(CPU_8MHz);

	// Setup the hardware
	init_hardware();

	// Print our static heading
	draw_string(0, 0, "TRANSMITTER:");
	show_screen();

	// Send a random character each time a button is released
	char buff[80], c;
	int seed = 0;
	while (1) {
		// If there's a button press, lets send a new character
		if (debounced_right_press()) {
			// Seed if necessary
			if (seed >= 0) {
				srand(seed);
				seed = -1;
			}

			// Generate the new character
			c = 'A' + rand() % 26;
			sprintf(buff, "Sending '%c'...", c);

			// Draw the new status of the system
			draw_string(0, 15, buff);
			show_screen();

			// Send the new character
			UART_send_char(c);
		}
		else if (seed >= 0) {
			seed++;
		}
	}

	// We'll never get here...
	return 0;
}

/*
* Function implementations
*/
void init_hardware(void) {
	// Initialising the LCD screen
	lcd_init(LCD_DEFAULT_CONTRAST);

	// Initalising the buttons as inputs
	DDRF &= ~((1 << PF5) | (1 << PF6));

	// Initialising the LEDs as outputs
	DDRB |= ((1 << PB2) | (1 << PB3));

	// Enable the transmission complete interrupt - this is how we locally give
	// you feedback that your UART transmission "WORKED". If you don't see your
	// LEDs toggling every time you send a character, then the interrupt isn't
	// running!
	// TODO
	UCSR1B |= ((1 << TXCIE1));

	// Set the UART prescaler so that we get the desired baud rate
	// TODO
	UBRR1 = BAUD_PRESCALER;        // set baud rate

	// Enable transmission of UART data
	// TODO
	UCSR1B |= ((1 << TXEN1)|(1<<RXEN1));// enable transmitter

	// Set the frame format to:
	// - Asynchronous normal mode
	// - 8 data bits
	// - 1 stop bit
	// - No parity bits
	// TODO
	// - Asynchronous normal mode
	UCSR1A &= ~(1 << U2X1);
	UCSR1C &= ~((1 << UMSEL11) | (1 << UMSEL10));
	// - 8 data bits
	UCSR1C |= ((1 << UCSZ10) | (1 << UCSZ11));
	UCSR1C &= ~((1 << UCSZ12));
	// - 1 stop bit
	UCSR1C &= ~((1 << USBS1));
	// - No parity bits
	UCSR1C &= ~((1 << UPM11) | (1 << UPM10));

	// Enable interrupts globally
	// TODO
	sei();
}

void UART_send_char(char data) {
	// Wait until the transmit buffer is empty
	// TODO
	while (!((UCSR1A >> UDRE1) & 1));	
	// Send the character
	// TODO
	UDR1 = data;
}

unsigned char debounced_right_press() {
	if ((PINF >> PF5) & 1) {
		_delay_ms(100);
		while ((PINF >> PF5) & 1);
		return 1;
	}
	else {
		return 0;
	}
}

/*
* Interrupt service routines
*/
ISR(USART1_TX_vect) {
	// Toggle the LEDs for some feedback...
	PORTB ^= (1 << PB2);
	PORTB ^= (1 << PB3);

	// Lets do some grossly non-verbose debugging checks to try help you out...
	if (
		*((unsigned char *)0xCD) == 0 &&
		*((unsigned char *)0xCC) == 0x33 &&
		(*((unsigned char *)0xC8) & 0b00000010) == 0 &&
		(*((unsigned char *)0xC9) & 0b01001100) == 0x48 &&
		(*((unsigned char *)0xCA) & 0b11111110) == 0x06
		) {
		draw_line(0, 30, LCD_X - 1, 30);
		draw_line(0, LCD_Y - 1, LCD_X - 1, LCD_Y - 1);
		draw_line(0, 30, 0, LCD_Y - 1);
		draw_line(LCD_X - 1, 30, LCD_X - 1, LCD_Y - 1);

		draw_line(0, 40, 18, 40);
		draw_line(18, 30, 18, 40);

		draw_string(2, 32, "DBG");
		draw_string(42, 38, "S E N T");

		// DO NOT DO THIS IN AN INTERRUPT - EVER!!!
		show_screen();
	}
}
