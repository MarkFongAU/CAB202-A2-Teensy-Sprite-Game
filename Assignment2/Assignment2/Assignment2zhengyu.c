/*
*  CAB202 Tutorial 9: Debouncing, timers, & interrupts
*	Question 4 - Template
*
*	B.Talbot, September 2015
*	Queensland University of Technology
*/
#define __AVR_ATmega32U4__ 
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>


#include "usb_serial.h"
#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "sprite.h"


/**
* This define controls whether button pins are initialised for the old or new
* Teensy. You must change this based on which version you have!
*/
#define AM_I_OLD 1

/**
* Useful defines you can use in your system time calculations
*/
#define FREQUENCY 8000000.0
#define PRESCALER 1024.0

uint8_t score = 0;
uint8_t live = 3;
uint8_t gotomain = 0;
uint8_t face_finish = 0;
uint8_t face_loop = 0;
uint8_t face_finish2 = 0;
uint8_t face_finish3 = 0;
uint8_t try1 = 0;
uint8_t try2 = 0;
uint8_t gameover = 0;
uint8_t face_speed = 6;
uint8_t level = 0;
uint16_t adc_result0;
uint8_t counter_for_speed = 0;
uint8_t num_of_face = 3;


int Pressed;
int press_count;
volatile int buttonPressed = 0;
float curr_time;
int Pressed_Confidence_Level = 0; //Measure button press cofidence
int Released_Confidence_Level = 0; //Measure button release confidence
int press_check = 1;
float face_x = 0;
float face2_x = 0;
float face3_x = 0;
float face_y = 9;
float player_x = 42;
char score_c[5];
char level_c[5];
char user_input;
float angle;
float l3_dx;
float l3_dy;
float l3_dx2;
float l3_dy2;
float l3_dx3;
float l3_dy3;

#define NUM_FACES 3
#define BYTES_PER_FACE 32
unsigned char bm_faces[NUM_FACES][BYTES_PER_FACE] = {
	{ 0x3F, 0xFC, 0x40, 0x02, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x86, 0x61, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x84, 0x21, 0x83, 0xC1, 0x80, 0x01, 0x80, 0x01, 0x40, 0x02, 0x3F, 0xFC },
	{ 0x3F, 0xFC, 0x7F, 0xFE, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xF0, 0x0F, 0xCC, 0x33, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xCF, 0xF3, 0xC0, 0x03, 0xC0, 0x03, 0x7F, 0xFE, 0x3F, 0xFC },
	{ 0x3F, 0xFC, 0x40, 0x02, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x84, 0x01, 0x86, 0x19, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x87, 0x81, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x40, 0x02, 0x3F, 0xFC }
}; //happy -> angry -> mad
unsigned char player_bm[16] =
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

void init_hardware(void);
void check_press_debounced(void);
uint16_t adc_read(uint8_t ch);
void show_name(void);
void init_faces(void);
void move_faces(void);


// initialize adc
void adc_init()
{
	// AREF = AVcc
	ADMUX = (1 << REFS0);

	// ADC Enable and pre-scaler of 128
	// 8000000/128 = 62500
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

int main() {

	//setup things
	set_clock_speed(CPU_8MHz);
	init_hardware();
	srand(adc_read(4));

	Sprite faces[3];
	Sprite player;
	for (uint8_t i = 0; i<3; i++)
	{
		init_sprite(&faces[i], 0, 16, 16, 16, bm_faces[i]);
	}
	init_sprite(&player, 0, LCD_Y - 8, 8, 8, player_bm);
	angle = ((float)rand()) * 2 * M_PI / RAND_MAX;
	l3_dx = cos(angle);
	l3_dy = sin(angle);
	show_name();
	if (level == 3)
	{
		faces[0].is_visible = 0;
		faces[1].is_visible = 0;
		faces[2].is_visible = 0;
	}


	while (gameover == 0) {
		clear_screen();
		if (level == 1)
		{
			check_press_debounced();
		}
		else if (level == 2)
		{

			adc_result0 = adc_read(0);
			float max_adc = 1023.0;
			long max_lcd_adc = (adc_result0*(long)(LCD_X - 8)) / max_adc;
			player.x = (uint8_t)max_lcd_adc;
		}
		if (level == 3)
		{
			while (!usb_configured() || !usb_serial_get_control())
			{
				draw_string(17, 17, "Waiting for");
				draw_string(19, 24, "Serial...");
				show_screen();
			}
			clear_screen();
			user_input = usb_serial_getchar();
			usb_serial_flush_input();
			if (user_input == 'w' && player.y >= 10)
			{
				player.y -= 2;
			}
			else if (user_input == 'a' && player.x>0)
			{
				player.x -= 2;
			}
			else if (user_input == 'd' && player.x + 7 < 83)
			{
				player.x += 2;
			}
			else if (user_input == 's' && player.y + 7 < 47)
			{
				player.y += 2;
			}
			for (uint8_t b = 0; b <3; b++)
			{
				if (faces[b].is_visible == 0)
				{
					if (b == 0)
					{
						angle = ((float)rand()) * 2 * M_PI / RAND_MAX;
						l3_dx = cos(angle);
						l3_dy = sin(angle);
						faces[b].x = rand() % 68;
						faces[b].is_visible = 1;
						face_finish = 0;
					}

					else if (b == 1)
					{
						angle = ((float)rand()) * 2 * M_PI / RAND_MAX;
						l3_dx2 = cos(angle);
						l3_dy2 = sin(angle);
						faces[b].x = rand() % 68;
						faces[b].is_visible = 1;
						face_finish2 = 0;
					}

					else if (b == 2)
					{
						angle = ((float)rand()) * 2 * M_PI / RAND_MAX;
						l3_dx3 = cos(angle);
						l3_dy3 = sin(angle);
						faces[b].x = rand() % 68;
						faces[b].is_visible = 1;
						face_finish3 = 0;
					}

				}

			}

			faces[0].x += l3_dx;
			faces[0].y += l3_dy;
			faces[1].x += l3_dx2;
			faces[1].y += l3_dy2;
			faces[2].x += l3_dx3;
			faces[2].y += l3_dy3;

			if ((uint8_t)faces[0].x >= LCD_X - 16 || (uint8_t)faces[0].x < 0)
			{
				l3_dx = -l3_dx;
			}

			if ((uint8_t)faces[0].y >= LCD_Y - 16 || (uint8_t)faces[0].y < 9)
			{
				l3_dy = -l3_dy;
			}

			if ((uint8_t)faces[1].x >= LCD_X - 16 || (uint8_t)faces[1].x < 0)
			{
				l3_dx2 = -l3_dx2;
			}

			if ((uint8_t)faces[1].y >= LCD_Y - 16 || (uint8_t)faces[1].y < 9)
			{
				l3_dy2 = -l3_dy2;
			}


			if ((uint8_t)faces[2].x >= LCD_X - 16 || (uint8_t)faces[2].x < 0)
			{
				l3_dx3 = -l3_dx3;
			}

			if ((uint8_t)faces[2].y >= LCD_Y - 16 || (uint8_t)faces[2].y < 9)
			{
				l3_dy3 = -l3_dy3;
			}






		}
		if (level == 1 || level == 2)
		{
			init_faces();
			faces[0].x = face_x;
			faces[0].y = face_y;
			faces[1].x = face2_x;
			faces[1].y = face_y;
			faces[2].x = face3_x;
			faces[2].y = face_y;
		}
		if (level == 1)
		{
			player.x = player_x;
		}
		for (uint8_t j = 0; j<3; j++)
		{
			draw_sprite(&faces[j]);
		}
		draw_sprite(&player);

		//detect faces 
		for (uint8_t k = 0; k<3; k++)
		{
			if (((uint8_t)player.x >= (uint8_t)faces[k].x && (uint8_t)player.x <= ((uint8_t)faces[k].x + 15)) || (((uint8_t)player.x + 7) >= (uint8_t)faces[k].x && ((uint8_t)player.x + 7) <= ((uint8_t)faces[k].x + 15)))
			{
				//Now we look at the y axis:
				if (((uint8_t)player.y >= (uint8_t)faces[k].y && (uint8_t)player.y <= ((uint8_t)faces[k].y + 15)) || (((uint8_t)player.y + 7) >= (uint8_t)faces[k].y && ((uint8_t)player.y + 7) <= ((uint8_t)faces[k].y + 15)))
				{
					//The sprites appear to overlap.
					if (k == 0)
					{
						if (face_finish == 0)
						{
							face_finish = 1;
							faces[0].is_visible = 0;
							score += 2;
							num_of_face--;
						}
					}
					else if (k == 1)
					{
						if (face_finish2 == 0)
						{
							face_finish2 = 1;
							faces[1].is_visible = 0;
							live--;
							num_of_face--;
						}
					}
					else if (k == 2)
					{
						if (face_finish3 == 0)
						{
							face_finish3 = 1;
							faces[2].is_visible = 0;
							if (face_speed >2)
							{
								face_speed -= 2;
							}
							num_of_face--;
						}
					}

				}
			}
		}

		if ((uint8_t)face_y >= LCD_Y || num_of_face <= 0)
		{
			face_y = 9;
			face_finish = 0;
			face_finish2 = 0;
			face_finish3 = 0;
			face_loop = 0;
			try2 = 0;
			num_of_face = 3;
			for (uint8_t a = 0; a<3; a++)
			{
				faces[a].is_visible = 1;
			}
		}

		draw_string(0, 0, "L: ");
		draw_char(10, 0, live + '0');
		draw_string(20, 0, "S: ");
		sprintf(score_c, "%d", score);
		draw_string(30, 0, score_c);
		draw_line(0, 8, 83, 8);


		show_screen();
		if (live == 0 || score == 20)

		{
			gameover = 1;
		}
	}
	clear_screen();
	draw_string(20, 20, "game over");
	show_screen();
	return 0;
}


void init_hardware(void) {

	adc_init();

	// Initialising the LCD screen
	LCDInitialise(LCD_DEFAULT_CONTRAST);
	


	// Initialise the USB serial
	usb_init();

	// Initalising the buttons as inputs (old and new)
	if (AM_I_OLD) {
		DDRB &= ~((1 << PB0) | (1 << PB1));
	}
	else {
		DDRF &= ~((1 << PF5) | (1 << PF6));
	}

	//interupt for y movement at 0.06sec
	TCCR1B |= (1 << CS11);
	TIMSK1 = (1 << TOIE1);
	// Enable global interrupts
	sei();
}

void check_press_debounced(void) {
	// A button press should only be registered ONCE every time that the
	// button is RELEASED.
	// TODO


	if (bit_is_clear(PINF, 5))
	{
		if (Pressed == 0)
		{
			Pressed = 1;
			if (player_x>0)
			{
				player_x -= 2;
			}
		}
	}
	else
		Pressed = 0;

	if (bit_is_clear(PINF, 6))
	{
		if (Pressed == 0)
		{
			Pressed = 1;
			if (player_x + 7 < 83)
			{
				player_x += 2;
			}
		}
	}

	else
		Pressed = 0;


}

void check_left_menu(void)
{
	if (bit_is_clear(PINF, 6))
	{
		if (Pressed == 0)
		{

			Pressed = 1;
			gotomain++;

		}
	}
	else
		Pressed = 0;


}

void check_right_menu(void) {
	// A button press should only be registered ONCE every time that the
	// button is RELEASED.
	// TODO

	// Counter for number of equal states
	static uint8_t count = 0;
	// Keeps track of current (debounced) state
	static uint8_t button_state = 0;
	// Check if button is high or low for the moment
	uint8_t current_state = (~PINF & (1 << PINF5)) != 0;
	if (current_state != button_state) {
		// Button state is about to be changed, increase counter
		count++;
		if (count >= 4) {
			// The button have not bounced for four checks, change state
			button_state = current_state;
			// If the button was pressed (not released), tell main so
			if (current_state != 0) {
				level++;
			}
			count = 0;
		}
	}
	else {
		// Reset counter
		count = 0;
	}


}
uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8) | ch;     // clears the bottom 3 bits before ORing

									 // start single conversion
									 // write '1' to ADSC
	ADCSRA |= (1 << ADSC);

	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while (ADCSRA & (1 << ADSC));

	return (ADC);
}


void show_name(void)
{
	while (gotomain <= 1)
	{
		draw_string(0, 0, "Lu Chen Hua");
		draw_string(0, 10, "N9543503");
		draw_string(0, 20, "Select Level: ");
		sprintf(level_c, "%d", level);
		draw_string(65, 20, level_c);
		show_screen();
		check_left_menu();
		check_right_menu();
		if (level>3)
		{
			level = 1;
		}

	}
}


void init_faces(void)
{
	if (face_loop == 0)
	{
	rerandom:
		face_x = rand() % 68;
		face2_x = rand() % 68;
		face3_x = rand() % 68;
		//width + 5px space
		while ((face2_x >= face_x - 5 && face2_x <= (face_x + 20)) ||
			((face2_x + 15) >= face_x - 5 && (face2_x + 15) <= (face_x + 20)))
		{
			face2_x = rand() % 68;
		}

		while ((face3_x >= face_x - 5 && face3_x <= (face_x + 20)) ||
			((face3_x + 15) >= face_x - 5 && (face3_x + 15) <= (face_x + 20)) ||

			(face3_x >= face2_x - 5 && face3_x <= (face2_x + 20)) ||
			((face3_x + 15) >= face2_x - 5 && (face3_x + 15) <= (face2_x + 20))

			)
		{
			face3_x = rand() % 68;
			try2++;
			if (try2>50)
			{
				try2 = 0;
				goto rerandom;

			}
		}
		face_loop = 1;
	}
}



ISR(TIMER1_OVF_vect) {

	if (gotomain>1)
	{
		counter_for_speed++;
		if (counter_for_speed >= face_speed)
		{
			if (level == 1 || level == 2)
			{

				face_y += 1.33;
			}

			else if (level == 3)
			{

			}
			counter_for_speed = 0;
		}
	}

}