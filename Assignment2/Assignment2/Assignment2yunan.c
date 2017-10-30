/*

*  CAB202 Tutorial 8: Display

*	frowny.c (Question4)

*

*	B.Talbot, September 2015

*	Queensland University of Technology

*/
#define __AVR_ATmega32U4__ 
#include <avr/io.h>

#include <util/delay.h>
#include <math.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "lcd.h"
#include "cpu_speed.h"
#include "graphics.h"
#include "sprite.h"
#include "usb_serial.h"


void init_hardware(void);
void init_game(void);
void start_game(void);
void button_init(void);
void display_level(void);
void display_status(void);
void show_name_stuent_number(void);
int collision_detect(Sprite *s1, Sprite *s2);
void set_random_position(int x[]);
uint16_t adc_read(uint8_t ch);
void game_over();
void spawn_face_hero(Sprite *a, Sprite *b, Sprite *c, Sprite *d);
int sprite_too_close(Sprite *a, Sprite *b);

// Useful defines
#define BIT(x) (1<<x)
#define INV_BIT(byte,bit) (byte^(1<<bit))


#define NUM_FACES 3
#define SMILE_FACE 0

#define BYTES_PER_FACE 32





// Array of bitmaps (i.e. an array of unsigned char arrays) that are used as the

// different sprite emotions

unsigned char bm_faces[NUM_FACES][BYTES_PER_FACE] = {
	/*smile*/
	{
		0b00000111, 0b11100000,
		0b00011000, 0b00011000,
	0b00100000, 0b00000100,
	0b01000000, 0b00000010,
	0b01011000, 0b00011010,
	0b10011000, 0b00011001,
	0b10000000, 0b00000001,
	0b10000000, 0b00000001,
	0b10010000, 0b00001001,
	0b10010000, 0b00001001,
	0b10001000, 0b00010001,
	0b01000111, 0b11100010,
	0b01000000, 0b00000010,
	0b00100000, 0b00000100,
	0b00011000, 0b00011000,
	0b00000111, 0b11100000
	},
	/*mad*/
	{
		0b00000111, 0b11100000,
		0b00011000, 0b00011000,
	0b00100000, 0b00000100,
	0b01000000, 0b00000010,
	0b01011000, 0b00011010,
	0b10011000, 0b00011001,
	0b10000000, 0b00000001,
	0b10000000, 0b00000001,
	0b10000111, 0b11100001,
	0b10001000, 0b00010001,
	0b10010000, 0b00001001,
	0b01010000, 0b00001010,
	0b01000000, 0b00000010,
	0b00100000, 0b00000100,
	0b00011000, 0b00011000,
	0b00000111, 0b11100000

	},
	/*mad*/
	{
		0b00000111, 0b11100000,
		0b00011000, 0b00011000,
	0b00100000, 0b00000100,
	0b01000000, 0b00000010,
	0b10001000, 0b00010001,
	0b10000100, 0b00100001,
	0b10000010, 0b01000001,
	0b10000000, 0b00000001,
	0b10000000, 0b00000001,
	0b10000011, 0b11000001,
	0b10000100, 0b00100001,
	0b01001000, 0b00010010,
	0b01000000, 0b00000010,
	0b00100000, 0b00000100,
	0b00011000, 0b00011000,
	0b00000111, 0b11100000
	}
};

unsigned char bm_hero[] = {
	0b10001000,
	0b10001000,
	0b10001000,
	0b11111000,
	0b10001000,
	0b10001000,
	0b10001000
};



/**

* Main

LCD Screen: 48 X 84

each charactor 7 X 5
cpu clock 8Mhz
Timer

*/

int game_level;
int score;
int lives;
volatile int delay_timer;
volatile int speed_timer;

volatile unsigned char left_button_released = 0;
volatile int left_button_pressed_timer;
volatile int left_button_released_timer;
volatile int left_button_status;     //0 released  1 pressed


volatile unsigned char right_button_released = 0;
volatile int right_button_pressed_timer;
volatile int right_button_released_timer;
volatile int right_button_status;     //0 released  1 pressed

int main() {

	// Set the clock prescaler and initialise the screen

	set_clock_speed(CPU_8MHz);

	lcd_init(LCD_DEFAULT_CONTRAST);
	//LCDInitialise(LCD_DEFAULT_CONTRAST);
	button_init();
	init_hardware();




	init_game();

	//show student name and number
	show_name_stuent_number();
	show_screen();
	_delay_ms(2000);
	int previous_hero_x;
	int hero_x;
	hero_x = 40;


	while (1) {
		//right button to change game level
		//left button to start the game
		score = 0;
		lives = 3;
		clear_screen();
		display_level();
		show_screen();

		start_game();


		if (game_level == 3) {
			speed_timer = 30;
			while (!usb_configured() || !usb_serial_get_control())
			{
				if (speed_timer == 0) {
					break;
				}
			}
		}

		Sprite faces[3];
		Sprite hero;

		if (game_level == 1 || game_level == 2) {
			int face_x_position[3];
			set_random_position(face_x_position);

			init_sprite(&hero, hero_x, 41, 5, 7, bm_hero);
			init_sprite(&faces[0], face_x_position[0], 10, 16, 16, bm_faces[0]);
			init_sprite(&faces[1], face_x_position[1], 10, 16, 16, bm_faces[1]);
			init_sprite(&faces[2], face_x_position[2], 10, 16, 16, bm_faces[2]);
		}
		else {
			//level 3
			init_sprite(&hero, 0, 0, 5, 7, bm_hero);
			init_sprite(&faces[0], 0, 0, 16, 16, bm_faces[0]);
			init_sprite(&faces[1], 0, 0, 16, 16, bm_faces[1]);
			init_sprite(&faces[2], 0, 0, 16, 16, bm_faces[2]);
		}


		// Update the sprite face



		int face_y = 0;
		int speed = 500;
		speed_timer = speed;

		if (game_level == 1 || game_level == 2) {
			while (1) {
				clear_screen();

				if (speed_timer == 0) {
					speed_timer = speed;
					face_y++;
					if (face_y + 16 >= 48) {
						int face_x_position[3];
						set_random_position(face_x_position);
						face_y = 0;
						//hero_x = 40;
						faces[0].x = face_x_position[0];
						faces[1].x = face_x_position[1];
						faces[2].x = face_x_position[2];
						if (game_level == 1)
							hero.x = hero_x;
						//when face apaear again, show all 3
						faces[0].is_visible = 1;
						faces[1].is_visible = 1;
						faces[2].is_visible = 1;

					}
					faces[0].y = 10 + face_y;
					faces[1].y = 10 + face_y;
					faces[2].y = 10 + face_y;
				}


				if (game_level == 1) {
					//draw_string(40, 41, "H");
					if (left_button_released) {
						left_button_released = 0;
						if (hero_x > 0) {
							hero_x--;
							//init_sprite(&hero, hero_x, 41, 5, 7, bm_hero);
							hero.x = hero_x;
						}
					}

					if (right_button_released) {
						right_button_released = 0;
						if (hero_x < (84 - 5)) {
							hero_x++;
							//init_sprite(&hero, hero_x, 41, 5, 7, bm_hero);
							hero.x = hero_x;
						}
					}
				}
				else if (game_level == 2) {
					uint16_t adc_reading, adc_reading1;
					adc_reading = adc_read(0);
					adc_reading1 = adc_reading;

					if (adc_reading1 > adc_reading)
						adc_reading = adc_reading1;
					//char str[32];
					//sprintf(str, "A:%d", adc_reading);
					//draw_string(0, 0, str);

					if (adc_reading <= 100) {
						hero_x = 0;
					}
					else if (adc_reading >= 1000) {
						hero_x = 84 - 5;
					}
					else {
						hero_x = (uint32_t)(adc_reading - 100) * 79 / 900;
					}

					hero.x = hero_x;
					//}
				}



				draw_sprite(&faces[0]);
				draw_sprite(&faces[1]);
				draw_sprite(&faces[2]);
				draw_sprite(&hero);
				display_status();
				show_screen();


				//hero collided with smilery face, get 2 points
				if (collision_detect(&faces[0], &hero)) {
					score += 2;
					faces[0].is_visible = 0;
					if (score >= 20)
						break;
				}

				//hero collided with mad face, increasing falling speed
				if (collision_detect(&faces[1], &hero)) {
					faces[1].is_visible = 0;
					if (speed > 260) {
						speed -= 120;
					}
				}

				//hero collided with angry face, lost 1 life 
				if (collision_detect(&faces[2], &hero)) {
					if (lives <= 1)
						break;
					lives--;
					faces[2].is_visible = 0;
				}

			}
		}
		else {
			//game level 3;
			clear_screen();
			display_status();
			show_screen();
			spawn_face_hero(&faces[0], &faces[1], &faces[2], &hero);


			while (1) {
				clear_screen();

				if (speed_timer == 0) {
					speed_timer = speed;
					faces[0].x += faces[0].dx;

					if (faces[0].x < 0) {
						faces[0].dx = 0 - faces[0].dx;
						faces[0].x = 0;
					}

					if (faces[0].x >(84 - 16)) {
						faces[0].dx = 0 - faces[0].dx;
						faces[0].x = 84 - 16;
					}

					faces[0].y += faces[0].dy;

					if (faces[0].y < 9) {
						faces[0].dy = 0 - faces[0].dy;
						faces[0].y = 9;
					}

					if (faces[0].y >(48 - 16)) {
						faces[0].dy = 0 - faces[0].dy;
						faces[0].y = 48 - 16;
					}




					faces[1].x += faces[1].dx;
					faces[1].y += faces[1].dy;
					faces[2].x += faces[2].dx;
					faces[2].y += faces[2].dy;

					if (faces[1].x < 0) {
						faces[1].dx = 0 - faces[1].dx;
						faces[1].x = 0;
					}

					if (faces[1].x >(84 - 16)) {
						faces[1].dx = 0 - faces[1].dx;
						faces[1].x = 84 - 16;
					}


					if (faces[1].y < 9) {
						faces[1].dy = 0 - faces[1].dy;
						faces[1].y = 9;
					}

					if (faces[1].y >(48 - 16)) {
						faces[1].dy = 0 - faces[1].dy;
						faces[1].y = 48 - 16;
					}


					if (faces[2].x < 0) {
						faces[2].dx = 0 - faces[2].dx;
						faces[2].x = 0;
					}

					if (faces[2].x >(84 - 16)) {
						faces[2].dx = 0 - faces[2].dx;
						faces[2].x = 84 - 16;
					}


					if (faces[2].y < 9) {
						faces[2].dy = 0 - faces[2].dy;
						faces[2].y = 9;
					}

					if (faces[2].y >(48 - 16)) {
						faces[2].dy = 0 - faces[2].dy;
						faces[2].y = 48 - 16;
					}

				}



				char c;
				c = usb_serial_getchar();

				if (c == 'a') {
					if (hero.x > 0) {
						hero.x--;
					}
				}
				else if (c == 'd') {
					if (hero.x < (84 - 5)) {
						hero.x++;
					}
				}
				else if (c == 's') {
					if (hero.y < (48 - 7)) {
						hero.y++;
					}
				}
				else if (c == 'w') {
					if (hero.y > 9) {
						hero.y--;
					}
				}


				if (game_level == 1) {
					//draw_string(40, 41, "H");
					if (left_button_released) {
						left_button_released = 0;
						if (hero_x > 0) {
							hero_x--;
							//init_sprite(&hero, hero_x, 41, 5, 7, bm_hero);
							hero.x = hero_x;
						}
					}

					if (right_button_released) {
						right_button_released = 0;
						if (hero_x < (84 - 5)) {
							hero_x++;
							//init_sprite(&hero, hero_x, 41, 5, 7, bm_hero);
							hero.x = hero_x;
						}
					}
				}
				else if (game_level == 2) {
					uint16_t adc_reading, adc_reading1;
					adc_reading = adc_read(0);
					adc_reading1 = adc_reading;

					if (adc_reading1 > adc_reading)
						adc_reading = adc_reading1;
					//char str[32];
					//sprintf(str, "A:%d", adc_reading);
					//draw_string(0, 0, str);

					if (adc_reading <= 100) {
						hero_x = 0;
					}
					else if (adc_reading >= 1000) {
						hero_x = 84 - 5;
					}
					else {
						hero_x = (uint32_t)(adc_reading - 100) * 79 / 900;
					}

					//if((previous_hero_x == -1) || (abs(hero_x - previous_hero_x) < 20)) {
					hero.x = hero_x;
					previous_hero_x = hero_x;
					//}
				}


				if (collision_detect(&faces[0], &faces[1])) {
					if (abs(faces[0].x - faces[1].x) <= 16) {
						faces[0].dx = 0 - faces[0].dx;
						faces[1].dx = 0 - faces[1].dx;
					}

					else if (abs(faces[0].y - faces[1].y) <= 16) {
						faces[0].dy = 0 - faces[0].dy;
						faces[1].dy = 0 - faces[1].dy;
					}
				}


				if (collision_detect(&faces[0], &faces[2])) {
					if (abs(faces[0].x - faces[2].x) <= 16) {
						faces[0].dx = 0 - faces[0].dx;
						faces[2].dx = 0 - faces[2].dx;
					}

					else if (abs(faces[0].y - faces[2].y) <= 16) {
						faces[0].dy = 0 - faces[0].dy;
						faces[2].dy = 0 - faces[2].dy;
					}
				}


				if (collision_detect(&faces[1], &faces[2])) {
					if (abs(faces[1].x - faces[2].x) <= 16) {
						faces[1].dx = 0 - faces[1].dx;
						faces[2].dx = 0 - faces[2].dx;
					}
					else if (abs(faces[1].y - faces[2].y) <= 16) {
						faces[1].dy = 0 - faces[1].dy;
						faces[2].dy = 0 - faces[2].dy;
					}
				}

				draw_sprite(&faces[0]);
				draw_sprite(&faces[1]);
				draw_sprite(&faces[2]);
				draw_sprite(&hero);
				display_status();
				show_screen();


				//hero collided with smilery face, get 2 points
				if (faces[0].is_visible && collision_detect(&faces[0], &hero)) {
					score += 2;
					faces[0].is_visible = 0;
					//spawn_face_hero(&faces[0], &faces[1], &faces[2], &hero); 

					if (score >= 20)
						break;
				}


				if (faces[0].is_visible == 0) {
					int retry;
					retry = 0;
					while (1) {
						if (retry++ > 100)
							break;
						srand(TCNT1);
						faces[0].x = rand() % (84 - 16);
						faces[0].y = (rand() % (48 - 9 - 16)) + 9;

						if (sprite_too_close(&faces[0], &faces[1]))
							continue;
						if (sprite_too_close(&faces[0], &faces[2]))
							continue;
						if (sprite_too_close(&faces[0], &hero))
							continue;

						//if get here, everything is ok, we get a good spawn
						faces[0].is_visible = 1;
						break;
					}
				}

				//hero collided with mad face, increasing falling speed
				if (faces[1].is_visible && collision_detect(&faces[1], &hero)) {

					faces[1].is_visible = 0;

					//spawn_face_hero(&faces[0], &faces[1], &faces[2], &hero); 

					if (speed > 260) {
						speed -= 120;
					}
				}

				if (faces[1].is_visible == 0) {
					int retry;
					retry = 0;
					while (1) {
						if (retry++ > 100)
							break;
						srand(TCNT1);
						faces[1].x = rand() % (84 - 16);
						faces[1].y = (rand() % (48 - 9 - 16)) + 9;

						if (sprite_too_close(&faces[1], &faces[0]))
							continue;
						if (sprite_too_close(&faces[1], &faces[2]))
							continue;
						if (sprite_too_close(&faces[1], &hero))
							continue;

						//if get here, everything is ok, we get a good spawn
						faces[1].is_visible = 1;
						break;
					}
				}

				//hero collided with angry face, lost 1 life 
				if (faces[2].is_visible && collision_detect(&faces[2], &hero)) {
					faces[2].is_visible = 0;
					if (lives <= 1)
						break;
					lives--;
					//spawn_face_hero(&faces[0], &faces[1], &faces[2], &hero);                    
				}


				if (faces[2].is_visible == 0) {
					int retry;
					retry = 0;
					while (1) {
						if (retry++ > 100)
							break;
						srand(TCNT1);
						faces[2].x = rand() % (84 - 16);
						faces[2].y = (rand() % (48 - 9 - 16)) + 9;

						if (sprite_too_close(&faces[2], &faces[0]))
							continue;
						if (sprite_too_close(&faces[2], &faces[1]))
							continue;
						if (sprite_too_close(&faces[2], &hero))
							continue;

						//if get here, everything is ok, we get a good spawn
						faces[2].is_visible = 1;
						break;
					}
				}



			}

		}


		// Set the face Id, create the sprite, and initialise it (provide it a null

		// pointer and let the main loop set the bitmap image)
		clear_screen();
		game_over();
		show_screen();
		_delay_ms(500);
		//left_button_released = 0;
		while (1) {
			if (left_button_released) {
				left_button_released = 0;
				break;;
			}
		}

	}

	//0 mad face
	//1 smile face


	// Draw sprite, show screen, rest
#if 0
	draw_sprite(&faces);

	while (1) {
		draw_sprite(&faces);
		show_screen();
#if 0
		delay_timer = 3000;
		PORTB |= (1 << 2);
		while (delay_timer);
		PORTB &= ~(1 << 2);
#else
		_delay_ms(3000);
#endif
		clear_screen();
	}
#endif

	// Run the face transition loop
#if 0
	while (1) {

		// Clear the screen

		clear_screen();



		// Update the sprite face

		faces.bitmap = bm_faces[face_id%NUM_FACES];



		// Draw sprite, show screen, rest

		draw_sprite(&faces);

		show_screen();

		_delay_ms(500);



		// Increment the sprite face, and have a break if we are back to the start

		face_id++;

		if (face_id%NUM_FACES == 0) {

			clear_screen();

			show_screen();

			_delay_ms(3000);

		}

	}
#endif


	// We'll never make it here!

	return 0;

}

void adc_init() {
	// AREF = AVcc
	ADMUX = (1 << REFS0);

	// ADC Enable and prescaler of 128
	// 8000000/128 = 62500
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with ?' will always keep the value
	// of ?h?between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8) | ch; // clears the bottom 3 bits before ORing

								 // start single convertion
								 // write ?' to ADSC
	ADCSRA |= (1 << ADSC);

	// wait for conversion to complete
	// ADSC becomes ?' again
	// till then, run loop continuously
	while (ADCSRA & (1 << ADSC));

	return (ADC);
}

void init_hardware() {
	// Initalising the buttons as inputs (new board)
	DDRF &= ~((1 << PF5) | (1 << PF6));

	// Initialising the LEDs as outputs
	DDRB |= ((1 << PB2) | (1 << PB3));

	// Setup TIMER1in "normal" operation mode
	TCCR1A = 0x00;
	// Set the prescaler for TIMER1 so that the clock overflows every ~2.1 seconds
	//TCCR1B = 0x00;
	//no preccaling, the timer clock source is cpu clock 8Mhz
	TCCR1B = 0x01;
	//TCCR1B = 0x05;     
	OCR1A = TCNT1 + 8000;
	// Enable the Timer Overflow Interrupt for TIMER1
	TIMSK1 |= (1 << OCIE1A);

	adc_init();
	// Enable global interrupts
	sei();
	
	PORTB &= ~(BIT(2) + BIT(3));

	// Initialise the USB serial
	usb_init();
}

void init_game() {
	game_level = 1;
	score = 0;
	lives = 3;
}


/* collision detection, for example, hero to mad face, smile face and angry face */
/* return 1 if it's collided, return 0 if it's not */
int collision_detect(Sprite *s1, Sprite *s2) {
	int x_overlap, y_overlap;
	x_overlap = 0;
	y_overlap = 0;

	//if one of them is not visible, impossible to collide
	if ((!s1->is_visible) || (!s2->is_visible)) {
		return 0;
	}

	if (s1->x >= s2->x) {
		if ((s1->x - s2->x) < s2->width) {
			x_overlap = 1;
		}
	}
	else {
		if ((s2->x - s1->x) < s1->width) {
			x_overlap = 1;
		}
	}

	if (s1->y >= s2->y) {
		if ((s1->y - s2->y) < s2->height) {
			y_overlap = 1;
		}
	}
	else {
		if ((s2->y - s1->y) < s1->height) {
			y_overlap = 1;
		}
	}

	if (x_overlap && y_overlap) {
		return 1;
	}
	else {
		return 0;
	}

}

int sprite_too_close(Sprite *a, Sprite *b) {

	if (a->x >= (b->x + b->width + 5))
		return 0;
	if (a->y >= (b->y + b->height + 5))
		return 0;
	if (b->x >= (a->x + a->width + 5))
		return 0;
	if (b->y >= (a->y + a->height + 5))
		return 0;

	return 1;

}


/* a, b, c faces, d is hero */
/* playing area x: 0 - 83, y: 9 - 47 */
void spawn_face_hero(Sprite *a, Sprite *b, Sprite *c, Sprite *d) {

	//spawn the faces and hero that meet the condition, no overlap,

	struct dx_dy {
		int dx;
		int dy;
	};


	struct dx_dy dx[] = {
		{ 0,1 },
		{ 1,0 },
		{ -1,-1 },
		{ 1,1 },
		{ -1,0 },
		{ 0,-1 },
		{ -1,1 },
		{ 1,-1 }
	};


	while (1) {
		srand(TCNT1);

		a->x = rand() % (84 - 16);
		b->x = rand() % (84 - 16);
		c->x = rand() % (84 - 16);
		d->x = rand() % (84 - 5);

		a->y = (rand() % (48 - 9 - 16)) + 9;
		b->y = (rand() % (48 - 9 - 16)) + 9;
		c->y = (rand() % (48 - 9 - 16)) + 9;
		d->y = (rand() % (48 - 9 - 7)) + 9;



		if (sprite_too_close(a, b))
			continue;
		if (sprite_too_close(a, c))
			continue;
		if (sprite_too_close(a, d))
			continue;
		if (sprite_too_close(b, c))
			continue;
		if (sprite_too_close(b, d))
			continue;
		if (sprite_too_close(c, d))
			continue;

		//if get here, everything is ok, we get a good spawn
		break;
	}

	int i;
	i = rand() % 8;
	a->dx = dx[i].dx;
	a->dy = dx[i].dy;

	i = rand() % 8;
	b->dx = dx[i].dx;
	b->dy = dx[i].dy;

	i = rand() % 8;
	c->dx = dx[i].dx;
	c->dy = dx[i].dy;
}

void set_random_position(int x[]) {
	int flag;
	while (1) {
		flag = 1;
		//set rand number seed
		srand(TCNT1);
		x[0] = rand() % (84 - 16);
		x[1] = rand() % (84 - 16);
		x[2] = rand() % (84 - 16);


		if (x[0] >= x[1] && x[0] < x[1] + 16 + 5) {
			flag = 0;
		}
		if (x[1] >= x[0] && x[1] < x[0] + 16 + 5) {
			flag = 0;
		}


		if (x[0] >= x[2] && x[0] < x[2] + 16 + 5) {
			flag = 0;
		}
		if (x[2] >= x[0] && x[2] < x[0] + 16 + 5) {
			flag = 0;
		}

		if (x[1] >= x[2] && x[1] < x[2] + 16 + 5) {
			flag = 0;
		}
		if (x[2] >= x[1] && x[2] < x[1] + 16 + 5) {
			flag = 0;
		}

		if (flag)
			break;
	}

}

void show_name_stuent_number() {

	// display name, centre
	draw_string(32, 0, "Name");
	draw_string(17, 8, "Yunan Wang");

	// discplay student Number
	draw_string(7, 16, "Student Number");
	draw_string(22, 24, "N8965919");
}


//display game level, default is level 1
void display_level() {
	char str[16];
	sprintf(str, "Game Level: %d", game_level);
	draw_string(0, 32, "                ");
	draw_string(9, 32, str);
}


//display game level, default is level 1
void game_over() {
	draw_string(9, 32, "Game Over");
}

//display status, current level, scores
void display_status(void) {
	char str[16];
	sprintf(str, "L: %d, S: %d", lives, score);
	draw_string(0, 0, "                ");
	draw_string(5, 0, str);
	draw_line(5, 8, 78, 8);
}



void button_init(void) {
	left_button_released = 0;
	left_button_pressed_timer = 0;
	left_button_released_timer = 0;
	left_button_status = 0;

	right_button_released = 0;
	right_button_pressed_timer = 0;
	right_button_released_timer = 0;
	right_button_status = 0;
}

void start_game(void) {
	while (1) {
		if (left_button_released) {
			left_button_released = 0;
			return;
		}
		if (right_button_released) {
			right_button_released = 0;
			game_level++;
			if (game_level >= 4)
				game_level = 1;
			display_level();
			show_screen();
		}

	}
}


int second = 0;
/**
* Interrupt service routines
* it interrupt every 1 million second
*/
ISR(TIMER1_COMPA_vect) {
	// Interrupt service routine for TIMER1. Toggle an LED everytime this ISR runs
	// TODO    
	OCR1A += 8000;
	//    PORTB = INV_BIT(PORTB,2);
	//    PORTB = INV_BIT(PORTB,3);

	//check if button right is pressed and released 
	if (PINF &  BIT(5)) {  //pressed
		right_button_pressed_timer++;
		right_button_released_timer = 0;

		if (right_button_pressed_timer > 50) {

			right_button_status = 1;
		}
	}
	else {
		right_button_pressed_timer = 0;
		right_button_released_timer++;

		if (right_button_released_timer > 50) {
			if (right_button_status == 1) {
				right_button_released = 1;
				//PORTB &= ~(1<<3); 
			}
			right_button_status = 0;
		}
	}


	//check if button left is pressed and released


	if (PINF &  BIT(6)) {  //pressed
		left_button_pressed_timer++;
		left_button_released_timer = 0;

		if (left_button_pressed_timer > 50) {

			left_button_status = 1;
		}
	}
	else {
		left_button_pressed_timer = 0;
		left_button_released_timer++;

		if (left_button_released_timer > 50) {
			if (left_button_status == 1) {
				left_button_released = 1;
				//PORTB &= ~(1<<3); 
			}
			left_button_status = 0;
		}
	}


	if (delay_timer)
		delay_timer--;

	if (speed_timer)
		speed_timer--;


}





