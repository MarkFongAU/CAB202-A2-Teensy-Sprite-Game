//#define __AVR_ATmega32U4__
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "sprite.h"
#include <stdbool.h>

//GAME DEFINES
#define BYTES_PER_ALIEN 3
#define BYTES_PER_PLAYER 4
#define BYTES_PER_WALL_BLOCK 8
#define BYTES_PER_ALIEN_BULLET 3
#define BYTES_PER_PLAYER_BULLET 1

#define NUM_ALIENS 15
#define NUM_WALLS 3
#define NUM_WALLS_BLOCK 12
#define MAX_BULLET_COUNT_ALIEN 5
#define MAX_BULLET_COUNT_PLAYER 15
#define ALIEN_DX 1
#define ALIEN_DY 1
#define ALIEN_BULLET_DY 2
#define PLAYER_DX 2
#define PLAYER_BULLET_DY 3

#define ALIEN_TIMER 500
#define ALIEN_BULLET_TIMER 500
#define ALIEN_BULLET_LOOP_DELAY 2000
#define PLAYER_TIMER 500
#define PLAYER_BULLET_TIMER 100
#define PLAYER_BULLET_X_TIMER 500

//BUTTON DEFINES
#define NUM_BUTTONS 6
#define BTN_DPAD_LEFT 0
#define BTN_DPAD_RIGHT 1
#define BTN_DPAD_UP 2
#define BTN_DPAD_DOWN 3
#define BTN_LEFT 4
#define BTN_RIGHT 5

#define BTN_STATE_UP 0
#define BTN_STATE_DOWN 1

#define BTN_PRESSED 1
#define BTN_NOT_PRESSED 0

//TIMER DEFINES
#define FREQUENCY 8000000.0
#define PRESCALER5 1024.0
#define PRESCALER4 256.0
#define PRESCALER3 64.0
#define PRESCALER2 8.0
#define PRESCALER1 1.0

/*
* Variables used in recording the state of the buttons. Note the use of volatile
* keyword. For arrays this keyword isn't necessary (consider why...), but we leave
* it there to show how to use volatile Sprite variables with the "cab202_teensy"
* library
*/
volatile Sprite btn_sprites[NUM_BUTTONS];
volatile unsigned char btn_hists[NUM_BUTTONS];
volatile unsigned char btn_states[NUM_BUTTONS];
volatile unsigned char btn_pressed[NUM_BUTTONS]; //0 not pressed, 1 pressed
volatile unsigned int press_count = 0;


// Array of bitmaps (i.e. an array of unsigned char arrays)
// Alien sprite
unsigned char bm_alien[BYTES_PER_ALIEN] = {
	0b00011000,
	0b11111111,
	0b01000010
};

// Player sprite
unsigned char bm_player[BYTES_PER_PLAYER] = {
	0b00011000,
	0b10111101,
	0b11111111,
	0b01100110
};

//Wall block sprite
unsigned char bm_wall_block[BYTES_PER_WALL_BLOCK] = {
	0b10000000
};

//Alien Bullet sprite
unsigned char bm_alien_bullet[BYTES_PER_ALIEN_BULLET] = {
	0b10000000,
	0b10000000,
	0b10000000
};

//Player Bullet sprite
unsigned char bm_player_bullet[BYTES_PER_PLAYER_BULLET] = {
	0b10000000,
};

int game_level;
int score;
int lives;
uint8_t sprite_started = 0;
int alien_bullet_count = 0;
int player_bullet_count = 0;
uint8_t shooting = 0;
float max_adc;
long max_lcd_adc;

volatile int delay_timer;
volatile int speed_timer;
volatile int alien_timer;
volatile int alien_bullet_timer;
volatile int alien_bullet_loop_delay;
volatile int player_timer;
volatile int player_bullet_timer;
volatile int player_bullet_x_timer[MAX_BULLET_COUNT_PLAYER];

int player_curvature;
uint8_t player_curvature_direction;
char score_c[5];

void init_hardware(void);
void adc_init();
void init_game(void);
void init_alien_location(Sprite *alien);
void init_wall_location(Sprite *wall, uint8_t location);
void init_player_location(Sprite *player);
void init_alien_bullet_location(Sprite *alien_bullet);
void init_player_bullet_location(Sprite *player_bullet);

void move_alien_location(Sprite *alien);
void check_pressed_debounce_game(Sprite *player);
void ADC_pressed_game_0(void);
void ADC_pressed_game_1(Sprite *player);

void setup_alien_bullet(Sprite *alien_bullet);
void launch_alien_bullet(Sprite *alien, Sprite *alien_bullet);
uint8_t move_alien_bullet(Sprite *alien_bullet, Sprite *player);
uint8_t move_alien_bullet_wall(Sprite *alien_bullet, Sprite *wall);
uint8_t update_alien_bullet_1(Sprite *alien, Sprite *alien_bullet, Sprite *player);
uint8_t update_alien_bullet_2(Sprite *alien, Sprite *alien_bullet, Sprite *player, Sprite *wall1, Sprite *wall2, Sprite *wall3);

void setup_player_bullet(Sprite *player_bullet);
void launch_player_bullet(Sprite *player, Sprite *player_bullet);
uint8_t move_player_bullet(Sprite *player_bullet, Sprite *alien, uint8_t current);
uint8_t move_player_bullet_wall(Sprite *player_bullet, Sprite *alien);
uint8_t update_player_bullet_1(Sprite *alien, Sprite *player_bullet, Sprite *player);
uint8_t update_player_bullet_2(Sprite *alien, Sprite *player_bullet, Sprite *player, Sprite *wall1, Sprite *wall2, Sprite *wall3);

void button_init(void);
void display_level(void);
void display_status(void);

void check_button_menu(void);

uint16_t adc_read(uint8_t ch);
void game_over();


//LCD Screen : 48 X 84
int main(void) {

	// Set the CPU speed to 8MHz (you must also be compiling at 8MHz)
	set_clock_speed(CPU_8MHz);
	//Initialize screen + hardware
	init_hardware();

	srand(adc_read(4));

	//Initilaize sprites
	Sprite alien[NUM_ALIENS];
	Sprite alien_bullet[MAX_BULLET_COUNT_ALIEN];
	Sprite wall1[NUM_WALLS_BLOCK];
	Sprite wall2[NUM_WALLS_BLOCK];
	Sprite wall3[NUM_WALLS_BLOCK];
	Sprite player;
	Sprite player_bullet[MAX_BULLET_COUNT_PLAYER];

	//Rows of aliens
	for (uint8_t i = 0; i < 5; i++)
	{
		init_sprite(&alien[i], 0, 16, 8, 3, bm_alien);
	}
	for (uint8_t i = 5; i < 10; i++)
	{
		init_sprite(&alien[i], 0, 16, 8, 3, bm_alien);
	}
	for (uint8_t i = 10; i < 15; i++)
	{
		init_sprite(&alien[i], 0, 16, 8, 3, bm_alien);
	}
	//Player
	init_sprite(&player, 0, LCD_Y - 8, 8, 4, bm_player);
	setup_alien_bullet(alien_bullet);
	setup_player_bullet(player_bullet);
	show_screen();

	

	while (1) {
		//Left button to start the game, Right button to change game level
		game_level = 1;
		score = 0;
		lives = 3;
		clear_screen();
		display_level();
		show_screen();
		check_button_menu();
		clear_screen();

		alien_timer = ALIEN_TIMER;
		alien_bullet_timer = ALIEN_BULLET_TIMER;
		player_bullet_timer = PLAYER_BULLET_TIMER;
		//player_bullet_x_timer = PLAYER_BULLET_X_TIMER;
		for(uint8_t j = 0; j < MAX_BULLET_COUNT_PLAYER; j++)
		{
			player_bullet_x_timer[j] = PLAYER_BULLET_X_TIMER;
		}
		alien_bullet_count = 0;
		player_bullet_count = 0;
		if (game_level == 1)
		{
			while (1) {
				clear_screen();
				if (sprite_started == 0)
				{
					//Setup bullets
					setup_alien_bullet(alien_bullet);
					setup_player_bullet(player_bullet);
					init_alien_bullet_location(alien_bullet);
					init_player_bullet_location(player_bullet);
					//Rows of aliens
					init_alien_location(alien);
					//Player
					init_player_location(&player);
					for (uint8_t j = 0; j < NUM_ALIENS; j++)
					{
						alien[j].is_visible = true;
						draw_sprite(&alien[j]);
					}
					draw_sprite(&player);
					sprite_started = 1;
				}
				//Move player
				check_pressed_debounce_game(&player);
				//Move alien
				move_alien_location(alien);
				//Move alien bullet
				update_alien_bullet_1(alien, alien_bullet, &player);
				//Move player bullet
				update_player_bullet_1(alien, player_bullet, &player);

				for (uint8_t j = 0; j < NUM_ALIENS; j++)
				{
					draw_sprite(&alien[j]);
				}
				for (uint8_t j = 0; j < MAX_BULLET_COUNT_ALIEN; j++)
				{
					draw_sprite(&alien_bullet[j]);
				}
				for (uint8_t j = 0; j < MAX_BULLET_COUNT_PLAYER; j++)
				{
					draw_sprite(&player_bullet[j]);
				}
				draw_sprite(&player);
				display_status();
				show_screen();
				if (lives < 1 || score >= 15) {
					sprite_started = 0;
					break;
				}
			}
		}
		else if (game_level == 2)
		{
			while (1) {
				clear_screen();
				if (sprite_started == 0)
				{
					//Setup bullets
					setup_alien_bullet(alien_bullet);
					setup_player_bullet(player_bullet);
					init_alien_bullet_location(alien_bullet);
					init_player_bullet_location(player_bullet);
					//Rows of aliens
					init_alien_location(alien);
					//Player
					init_player_location(&player);
					//Rows of walls
					init_wall_location(wall1, 16);
					init_wall_location(wall2, 38);
					init_wall_location(wall3, 61);
					//Draw sprite
					for (uint8_t j = 0; j < NUM_ALIENS; j++)
					{
						alien[j].is_visible = true;
						draw_sprite(&alien[j]);
					}
					for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
					{
						wall1[j].is_visible = true;
						draw_sprite(&wall1[j]);
					}
					for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
					{
						wall2[j].is_visible = true;
						draw_sprite(&wall2[j]);
					}
					for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
					{
						wall3[j].is_visible = true;
						draw_sprite(&wall3[j]);
					}
					draw_sprite(&player);
					sprite_started = 1;
				}
				//Move player
				check_pressed_debounce_game(&player);
				ADC_pressed_game_1(&player);

				//Move alien
				move_alien_location(alien);
				//Move alien bullet
				//update_alien_bullet_1(alien, alien_bullet, &player);
				update_alien_bullet_2(alien, alien_bullet, &player, wall1, wall2, wall3);
				//Move player bullet
				//update_player_bullet_1(alien, player_bullet, &player);
				update_player_bullet_2(alien, player_bullet, &player, wall1, wall2, wall3);

				for (uint8_t j = 0; j < NUM_ALIENS; j++)
				{
					draw_sprite(&alien[j]);
				}
				for (uint8_t j = 0; j < MAX_BULLET_COUNT_ALIEN; j++)
				{
					draw_sprite(&alien_bullet[j]);
				}
				for (uint8_t j = 0; j < MAX_BULLET_COUNT_PLAYER; j++)
				{
					draw_sprite(&player_bullet[j]);
				}
				for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
				{
					draw_sprite(&wall1[j]);
				}
				for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
				{
					draw_sprite(&wall2[j]);
				}
				for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
				{
					draw_sprite(&wall3[j]);
				}
				draw_sprite(&player);
				display_status();
				show_screen();
				if (lives < 1 || score >= 15) {
					sprite_started = 0;
					break;
				}
			}
		}
		else if (game_level == 3)
		{
			while (1) {
				clear_screen();
				if (sprite_started == 0)
				{
					//Setup bullets
					setup_alien_bullet(alien_bullet);
					setup_player_bullet(player_bullet);
					init_alien_bullet_location(alien_bullet);
					init_player_bullet_location(player_bullet);
					//Rows of aliens
					init_alien_location(alien);
					//Player
					init_player_location(&player);
					//Rows of walls
					init_wall_location(wall1, 16);
					init_wall_location(wall2, 38);
					init_wall_location(wall3, 61);
					//Draw sprite
					for (uint8_t j = 0; j < NUM_ALIENS; j++)
					{
						alien[j].is_visible = true;
						draw_sprite(&alien[j]);
					}
					for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
					{
						wall1[j].is_visible = true;
						draw_sprite(&wall1[j]);
					}
					for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
					{
						wall2[j].is_visible = true;
						draw_sprite(&wall2[j]);
					}
					for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
					{
						wall3[j].is_visible = true;
						draw_sprite(&wall3[j]);
					}
					draw_sprite(&player);
					sprite_started = 1;
				}
				//Move player
				check_pressed_debounce_game(&player);
				ADC_pressed_game_1(&player);
				ADC_pressed_game_0();

				//Move alien
				move_alien_location(alien);
				//Move alien bullet
				//update_alien_bullet_1(alien, alien_bullet, &player);
				update_alien_bullet_2(alien, alien_bullet, &player, wall1, wall2, wall3);
				//Move player bullet
				//update_player_bullet_1(alien, player_bullet, &player);
				update_player_bullet_2(alien, player_bullet, &player, wall1, wall2, wall3);

				for (uint8_t j = 0; j < NUM_ALIENS; j++)
				{
					draw_sprite(&alien[j]);
				}
				for (uint8_t j = 0; j < MAX_BULLET_COUNT_ALIEN; j++)
				{
					draw_sprite(&alien_bullet[j]);
				}
				for (uint8_t j = 0; j < MAX_BULLET_COUNT_PLAYER; j++)
				{
					draw_sprite(&player_bullet[j]);
				}
				for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
				{
					draw_sprite(&wall1[j]);
				}
				for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
				{
					draw_sprite(&wall2[j]);
				}
				for (uint8_t j = 0; j < NUM_WALLS_BLOCK; j++)
				{
					draw_sprite(&wall3[j]);
				}
				draw_sprite(&player);
				display_status();
				show_screen();
				if (lives < 1 || score >= 15) {
					sprite_started = 0;
					break;
				}
			}
		}
		//End the game loop, back to main menu
		clear_screen();
		game_over();
		show_screen();
		_delay_ms(500);
	}
	return 0;
}
//Initliaze Alien
void init_alien_location(Sprite* alien)
{
	//Rows of aliens
	for (uint8_t i = 0; i < 5; i++)
	{
		alien[i].x = 18 + (i * 8) + (i * 2);
		alien[i].y = 10;
		alien[i].dx = 1;
		alien[i].dy = 0;
	}
	if(game_level == 3)
	{
		for (uint8_t i = 5; i < 10; i++)
		{
			alien[i].x = 18 + ((i - 5) * 8) + ((i - 5) * 2);
			alien[i].y = 13;
			alien[i].dx = -1;
			alien[i].dy = 0;
		}
	}else
	{
		for (uint8_t i = 5; i < 10; i++)
		{
			alien[i].x = 18 + ((i - 5) * 8) + ((i - 5) * 2);
			alien[i].y = 13;
			alien[i].dx = 1;
			alien[i].dy = 0;
		}
	}
	for (uint8_t i = 10; i < 15; i++)
	{
		alien[i].x = 18 + ((i - 10) * 8) + ((i - 10) * 2);
		alien[i].y = 16;
		alien[i].dx = 1;
		alien[i].dy = 0;
	}
}

//Move Alien
void move_alien_location(Sprite* alien)
{
	if (alien_timer == 0) {
		alien_timer = ALIEN_TIMER;
		for (uint8_t k = 0; k < NUM_ALIENS; k++) {
			if(alien[k].is_visible)
			{
				alien[k].x += alien[k].dx;
				alien[k].y += alien[k].dy;
			}
		}
		if (game_level == 1 || game_level == 2)
		{
			for(uint8_t l = 0; l < NUM_ALIENS; l++)
			{
				if (alien[l].is_visible)
				{
					//If alien row reach the right side of screen
					if ((uint8_t)alien[l].x + 7 >= 83) {
						for (uint8_t k = 0; k < NUM_ALIENS; k++) {
							alien[k].dx = -ALIEN_DX;
						}
						goto end1;
					}
					//If alien row reach the left side of screen
					else if ((uint8_t)alien[l].x <= 0)
					{
						for (uint8_t k = 0; k < NUM_ALIENS; k++) {
							alien[k].dx = ALIEN_DX;
						}
						goto end1;
					}
				}
			}
		end1:;
		}
		if (game_level == 3)
		{
			for (uint8_t k = 0; k < 5; k++)
			{
				if (alien[k].is_visible)
				{
					//If alien row reach the right side of screen
					if ((uint8_t)alien[k].x + 7 >= 83)
					{
						for (uint8_t l = 0; l < 5; l++) {
							alien[l].dx = -ALIEN_DX;
						}
						goto endrow1;
					}
					//If alien row reach the left side of screen
					else if((uint8_t)alien[k].x <= 0)
					{
						for (uint8_t l = 0; l < 5; l++) {
							alien[l].dx = ALIEN_DX;
						}
						goto endrow1;
					}

				}
			}
			endrow1:;
			for (uint8_t k = 5; k < 10; k++)
			{
				if (alien[k].is_visible)
				{
					//If alien row reach the right side of screen
					if ((uint8_t)alien[k].x + 7 >= 83)
					{
						for (uint8_t l = 5; l < 10; l++) {
							alien[l].dx = -ALIEN_DX;
						}
						goto endrow2;
					}
					//If alien row reach the left side of screen
					else if ((uint8_t)alien[k].x <= 0)
					{
						for (uint8_t l = 5; l < 10; l++) {
							alien[l].dx = ALIEN_DX;
						}
						goto endrow2;
					}

				}
			}
			endrow2:;
			for (uint8_t k = 10; k < 15; k++)
			{
				if (alien[k].is_visible)
				{
					//If alien row reach the right side of screen
					if ((uint8_t)alien[k].x + 7 >= 83)
					{
						for (uint8_t l = 10; l < 15; l++) {
							alien[l].dx = -ALIEN_DX;
						}
						goto endrow3;
					}
					//If alien row reach the left side of screen
					else if ((uint8_t)alien[k].x <= 0)
					{
						for (uint8_t l = 10; l < 15; l++) {
							alien[l].dx = ALIEN_DX;
						}
						goto endrow3;
					}

				}
			}
			endrow3:;
		}

		//Level 2 and 3
		if (game_level == 2 || game_level == 3)
		{
			for (uint8_t l = 0; l < NUM_ALIENS; l++)
			{
				if (alien[l].is_visible)
				{
					//If alien row reach the top side of screen, 10
					if ((uint8_t)alien[l].y <= 10) {
						for (uint8_t k = 0; k < NUM_ALIENS; k++) {
							alien[k].dy = ALIEN_DY;
						}
						goto end2;
					}
					//If alien row reach the middle of screen, 24
					else if ((uint8_t)alien[l].y +3 >= 24)
					{
						for (uint8_t k = 0; k < NUM_ALIENS; k++) {
							alien[k].dy = -ALIEN_DY;
						}
						goto end2;
					}
				}
			}
		end2:;
		}
	}
}
//Setup alien bullet
void setup_alien_bullet(Sprite *alien_bullet) {
	for (int i = 0; i < MAX_BULLET_COUNT_ALIEN; i++)
	{
		init_sprite(&alien_bullet[i], -1, -1, 1, 3, bm_alien_bullet);
		alien_bullet[i].is_visible = false;
	}
}

//Initialize alien bullet location
void init_alien_bullet_location(Sprite *alien_bullet)
{
	//Rows of aliens
	for (uint8_t i = 0; i < MAX_BULLET_COUNT_ALIEN; i++)
	{
		alien_bullet[i].x = -1;
		alien_bullet[i].y = -1;
		alien_bullet[i].is_visible = false;
	}
}

//Launch alien bullet
void launch_alien_bullet(Sprite *alien, Sprite *alien_bullet) {
	uint8_t x = rand() % NUM_ALIENS;

	while (!alien[x].is_visible)
	{
		x = rand() % NUM_ALIENS;
	}
	for (int i = 0; i < alien_bullet_count + 1; i++)
	{
		if (!alien_bullet[i].is_visible)
		{
			alien_bullet[i].x = alien[x].x + 3;
			alien_bullet[i].y = alien[x].y + 3;
			alien_bullet[i].is_visible = true;
		}
	}
}


//Move alien bullet
uint8_t move_alien_bullet(Sprite *alien_bullet, Sprite *player) {
	if (alien_bullet->is_visible)
	{
		alien_bullet->y = alien_bullet->y + 1;
		if ((uint8_t)alien_bullet->y >= 47) //If the coodinate y of alien_bullet > player then disappear
		{
			alien_bullet->x = -1;
			alien_bullet->y = -1;
			alien_bullet_count--;
			alien_bullet->is_visible = false;
		}

		if (( //If hit player
			(uint8_t)(alien_bullet->x + alien_bullet->width - 1) >= (uint8_t)player->x &&
			(uint8_t)(alien_bullet->x + alien_bullet->width - 1) <= (uint8_t)(player->x + player->width - 1) &&
			(uint8_t)(alien_bullet->y + alien_bullet->height - 1) >= (uint8_t)player->y ) ||//&&
			//(uint8_t)(alien_bullet->y + alien_bullet->height - 1) <= (uint8_t)(player->y + player->height-1)) ||
			(
				(uint8_t)alien_bullet->x >= (uint8_t)player->x &&
				(uint8_t)alien_bullet->x <= (uint8_t)(player->x + player->width - 1) &&
				(uint8_t)(alien_bullet->y + alien_bullet->height - 1) >= (uint8_t)player->y ))//&&
				//(uint8_t)(alien_bullet->y + alien_bullet->height - 1) <= (uint8_t)(player->y + player->height - 1)))
		{
			alien_bullet->is_visible = false;
			lives -= 1;
			alien_bullet->x = -1;
			alien_bullet->y = -1;
			alien_bullet_count--;
			player->x = 38;
			player->x = 44;
			player->is_visible = true;
		}
		return true;
	}
	return false;
}

//Move alien bullet
uint8_t move_alien_bullet_wall(Sprite *alien_bullet, Sprite *wall) {
	if (alien_bullet->is_visible)
	{
		for (int i = 0; i < NUM_WALLS_BLOCK; i++)
		{
			if (//If hit wall
				(uint8_t)(alien_bullet->x ) == (uint8_t)wall[i].x &&
				(uint8_t)(alien_bullet->y + alien_bullet->height) >= (uint8_t)wall[i].y )
			{
				alien_bullet->is_visible = false;
				alien_bullet->x = -1;
				alien_bullet->y = -1;
				alien_bullet_count--;
				wall[i].is_visible = false;
				wall[i].x = -1;
				wall[i].y = -1;
			}
		}
		return true;
	}
	return false;
}

//Update alien bullet Level 1
uint8_t update_alien_bullet_1(Sprite *alien, Sprite *alien_bullet, Sprite *player) {
	uint8_t changed = false;
	if (alien_bullet_count < 0)alien_bullet_count = 0;
	if (alien_bullet_count < MAX_BULLET_COUNT_ALIEN)
	{
		if (alien_bullet_loop_delay == 0)
		{
			alien_bullet_loop_delay = ALIEN_BULLET_LOOP_DELAY;
			launch_alien_bullet(alien, alien_bullet);
			alien_bullet_count++;
		}
	}
	if (alien_bullet_timer == 0)
	{
		alien_bullet_timer = ALIEN_BULLET_TIMER;
		for (int i = 0; i < MAX_BULLET_COUNT_ALIEN; i++)
		{
			changed = move_alien_bullet(&alien_bullet[i], player);
		}
	}
	return changed;
}
//Update alien bullet Level 2
uint8_t update_alien_bullet_2(Sprite *alien, Sprite *alien_bullet, Sprite *player, Sprite *wall1, Sprite *wall2, Sprite *wall3) {
	uint8_t changed = false;
	if (alien_bullet_count < 0)alien_bullet_count = 0;
	if (alien_bullet_count < MAX_BULLET_COUNT_ALIEN)
	{
		if (alien_bullet_loop_delay == 0)
		{
			alien_bullet_loop_delay = ALIEN_BULLET_LOOP_DELAY;
			launch_alien_bullet(alien, alien_bullet);
			alien_bullet_count++;
		}
	}
	if (alien_bullet_timer == 0)
	{
		alien_bullet_timer = ALIEN_BULLET_TIMER;
		for (int i = 0; i < MAX_BULLET_COUNT_ALIEN; i++)
		{
			changed = move_alien_bullet(&alien_bullet[i], player);
			move_alien_bullet_wall(&alien_bullet[i], wall1);
			move_alien_bullet_wall(&alien_bullet[i], wall2);
			move_alien_bullet_wall(&alien_bullet[i], wall3);
		}
	}
	return changed;
}
//Setup player bullet
void setup_player_bullet(Sprite *player_bullet) {
	for (int i = 0; i < MAX_BULLET_COUNT_PLAYER; i++)
	{
		init_sprite(&player_bullet[i], -1, -1, 1, 1, bm_player_bullet);
		player_bullet[i].is_visible = false;
	}
}

//Initialize player bullet location
void init_player_bullet_location(Sprite *player_bullet)
{
	//Rows of aliens
	for (uint8_t i = 0; i < MAX_BULLET_COUNT_PLAYER; i++)
	{
		player_bullet[i].x = -1;
		player_bullet[i].y = -1;
		player_bullet[i].is_visible = false;
	}
}

//Move player bullet
uint8_t move_player_bullet(Sprite *player_bullet, Sprite *alien, uint8_t current) {
	if (player_bullet->is_visible)
	{
		//Curvature X
		if(game_level == 3)
		{
			if(player_bullet_x_timer[current] <= 0)
			{
				player_bullet_x_timer[current] = player_curvature;
				if (player_curvature_direction == 0)player_bullet->x = player_bullet->x - 1;
				else if (player_curvature_direction == 1)player_bullet->x = player_bullet->x + 1;
			}
		}

		player_bullet->y = player_bullet->y - 1;
		if ((uint8_t)(player_bullet->y) < 9 || (uint8_t)(player_bullet->x < 0 ) || (uint8_t)(player_bullet->x >= LCD_X))
		{
			player_bullet->x = -1;
			player_bullet->y = -1;
			player_bullet_count--;
			player_bullet->is_visible = false;
		}
		else
		{
			for (int i = 0; i < NUM_ALIENS; i++)
			{
				if (
					(uint8_t)player_bullet->x >= (uint8_t)alien[i].x &&
					(uint8_t)player_bullet->x <= (uint8_t)(alien[i].x + alien[i].width - 1) &&
					(uint8_t)player_bullet->y <= (uint8_t)(alien[i].y + alien[i].height - 1) &&
					(uint8_t)player_bullet->y >= (uint8_t)alien[i].y)
				{
					player_bullet->is_visible = false;
					player_bullet->x = -1;
					player_bullet->y = -1;
					player_bullet_count--;
					alien[i].is_visible = false;
					alien[i].x = -1;
					alien[i].y = -1;
					score += 1;
				}
			}
		}
		return true;
	}
	return false;
}

//Move player bullet wall
uint8_t move_player_bullet_wall(Sprite *player_bullet, Sprite *wall) {
	if (player_bullet->is_visible)
	{
		for (int i = 0; i < NUM_WALLS_BLOCK; i++)
		{
			if(wall[i].is_visible)
			{
				if (
					(uint8_t)player_bullet->x == (uint8_t)wall[i].x &&
					(uint8_t)player_bullet->y == (uint8_t)wall[i].y)
				{
					player_bullet->is_visible = false;
					player_bullet->x = -1;
					player_bullet->y = -1;
					player_bullet_count--;
					wall[i].is_visible = false;
					wall[i].x = -1;
					wall[i].y = -1;
				}
			}
		}
		return true;
	}
	return false;
}
//Launch player bullet
void launch_player_bullet(Sprite *player, Sprite *player_bullet) {
	for (int i = 0; i < player_bullet_count + 1; i++)
	{
		if (!player_bullet[i].is_visible)
		{
			player_bullet[i].x = player->x + 3;
			player_bullet[i].y = player->y - 1;
			player_bullet[i].dy = -1;
			player_bullet[i].dx = 0;
			player_bullet[i].is_visible = true;
		}
	}
}

//Update player bullet Level 1
uint8_t update_player_bullet_1(Sprite *alien, Sprite *player_bullet, Sprite *player) {
	uint8_t changed = false;
	if (player_bullet_count < 0)player_bullet_count = 0;
	if (player_bullet_count < MAX_BULLET_COUNT_PLAYER)
	{
		if(shooting==1)
		{
			launch_player_bullet(player, player_bullet);
			player_bullet_count++;
			shooting = 0;
		}
	}

	if (player_bullet_timer == 0)
	{
		player_bullet_timer = PLAYER_BULLET_TIMER;
		for (int i = 0; i < MAX_BULLET_COUNT_PLAYER; i++)
		{
			changed = move_player_bullet(&player_bullet[i], alien, i);
		}
	}
	return changed;
}

//Update player bullet Level 2
uint8_t update_player_bullet_2(Sprite *alien, Sprite *player_bullet, Sprite *player, Sprite *wall1, Sprite *wall2, Sprite *wall3) {
	uint8_t changed = false;
	if (player_bullet_count < 0)player_bullet_count = 0;
	if (player_bullet_count < MAX_BULLET_COUNT_PLAYER)
	{
		if (shooting == 1)
		{
			launch_player_bullet(player, player_bullet);
			player_bullet_count++;
			shooting = 0;
		}
	}

	if (player_bullet_timer == 0)
	{
		player_bullet_timer = PLAYER_BULLET_TIMER;
		for (int i = 0; i < MAX_BULLET_COUNT_PLAYER; i++)
		{
			changed = move_player_bullet(&player_bullet[i], alien, i);
			move_player_bullet_wall(&player_bullet[i], wall1);
			move_player_bullet_wall(&player_bullet[i], wall2);
			move_player_bullet_wall(&player_bullet[i], wall3);
		}
	}
	return changed;
}

//Initliaze Player
void init_player_location(Sprite *s)
{
	s->x = 38;
	s->y = 44;
	s->dx = 0;
	s->dy = 0;
}

//Initliaze Walls
void init_wall_location(Sprite *wall, uint8_t location)
{
	//Rows of walls
	for (uint8_t i = 0; i < 6; i++)
	{
		init_sprite(&wall[i], location + i, 33, 1, 1, bm_wall_block);
	}
	for (uint8_t i = 6; i < 12; i++)
	{
		init_sprite(&wall[i], location + i - 6, 34, 1, 1, bm_wall_block);
	}
}

//Display game level, default is level 1
void display_level() {
	char str[16];
	sprintf(str, "Game Level: %d", game_level);
	draw_string(0, 24, "                ");
	draw_string(9, 24, str);
}

//Display message 'Game Over'
void game_over() {
	draw_string(9, 24, "Game Over");
}

//Display status, current level, scores
void display_status(void) {
	char str[16];
	sprintf(str, "S:%d, L:%d, Lv:%d", score, lives, game_level);
	draw_string(0, 0, "                 ");
	draw_string(0, 0, str);
	draw_line(0, 8, 83, 8);
}

//Check button in menu
void check_button_menu(void)
{
	while (1) {
		//Left
		if (!bit_is_clear(PINF, 6)) {
			_delay_ms(100);
			while ((PINF >> PF6) & 1);
			return;
		}
		//Right
		else if (!bit_is_clear(PINF, 5)) {
			_delay_ms(100);
			while ((PINF >> PF5) & 1);
			game_level++;
			if (game_level > 3)
			{
				game_level = 1;
			}
			display_level();
			show_screen();
		}
	}
}

//Check button in game
void check_pressed_debounce_game(Sprite *player) {
	//Left
	if(btn_states[BTN_LEFT] == BTN_STATE_DOWN)
	{
		if (player->x > 0) {
			player->x -= 1;
		}
	}
	//Right
	else if (btn_states[BTN_RIGHT] == BTN_STATE_DOWN)
	{
		if (player->x + 7 < 83) {
			player->x += 1;
		}
	}
	if (btn_pressed[BTN_DPAD_UP] == BTN_PRESSED)
	{
		shooting = 1;
		btn_pressed[BTN_DPAD_UP] = BTN_NOT_PRESSED;
	}
	//Level 3
	if(game_level==2 || game_level==3)
	{
		if (btn_states[BTN_DPAD_LEFT] == BTN_STATE_DOWN)
		{
			if (player->x > 0) {
				player->x -= 1;
			}
		}
		if (btn_states[BTN_DPAD_RIGHT] == BTN_STATE_DOWN)
		{
			if (player->x + 7 < 83) {
				player->x += 1;
			}
		}
	}
}
//Check potentionmeter ADC00 in game
void ADC_pressed_game_0(void)
{
	uint16_t adc_reading_0 = adc_read(0);

	max_adc = 1023.0;
	max_lcd_adc = adc_reading_0 * (long)1000 / max_adc;
	player_curvature = (int)max_lcd_adc;

	if (player_curvature < 500){
		player_curvature_direction = 0; //Left
		player_curvature = (int)(player_curvature + 100);
	}
	else if (player_curvature >500){
		player_curvature_direction = 1; //Right
		player_curvature = (int)(1100 - player_curvature);
	}
	else player_curvature_direction = 2; //No change in direction
}

//Check potentionmeter ADC01 in game
void ADC_pressed_game_1(Sprite *player)
{
	uint16_t adc_reading_1 = adc_read(1);

	max_adc = 1023.0;
	max_lcd_adc = (adc_reading_1 * (long)(LCD_X - player->width)) / max_adc;
	player->x = (int)max_lcd_adc;
}

//Initialize ADC
void adc_init()
{
	// AREF = AVcc
	ADMUX = (1 << REFS0);

	// ADC Enable and pre-scaler of 128
	// 8000000/128 = 62500
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

//Read ADC
uint16_t adc_read(uint8_t ch)
{
	// Select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8) | ch;     // Clears the bottom 3 bits before ORing

	// Start single conversion
	// Write '1' to ADSC
	ADCSRA |= (1 << ADSC);

	// Wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while (ADCSRA & (1 << ADSC));

	return (ADC);
}
//Initialize hardware and screen
void init_hardware(void) {
	//ADC potentiometer
	adc_init();

	// Initialising the LCD screen
	lcd_init(LCD_DEFAULT_CONTRAST);

	// Initalising the buttons as inputs
	// TODO
	DDRF &= ~((1 << PF5) | (1 << PF6));
	DDRB &= ~((1 << PB1) | (1 << PB7));
	DDRD &= ~((1 << PD0) | (1 << PD1));

	// Initialising the LEDs as outputs
	// TODO
	DDRB |= ((1 << PB2) | (1 << PB3));
	DDRC |= (1 << 7);

	// Configure all necessary timers in "normal mode", enable all necessary
	// interupts, and configure prescalers as desired
	// TODO
	TCCR0B &= ~((1 << WGM02));
	TCCR1B &= ~((1 << WGM02));

	//Timer 0 Overflow 2 millisecond
	TCCR0B |= ((1 << CS01) | (1 << CS00));
	TCCR0B &= ~((1 << CS02));

	TIMSK0 |= (1 << TOIE0);

	//Timer 1 Overflow 8 millisecond
	TCCR1B |= (1 << CS00);
	TCCR1B &= ~((1 << CS02) | (1 << CS01));

	TIMSK1 |= (1 << TOIE1);

	// Globally enable interrupts
	// TODO
	sei();
}

//Interrupt Routines
ISR(TIMER1_OVF_vect) {
	// Interrupt service routine for TIMER0. Toggle an LED everytime this ISR runs
	// TODO
	//PORTC |= (1 << 7);
	for (unsigned char i = 0; i < NUM_BUTTONS; i++) {
		btn_hists[i] = (btn_hists[i] << 1);
	}

	btn_hists[BTN_LEFT] |= ((PINF >> PF6) & 1) << 0;
	btn_hists[BTN_RIGHT] |= ((PINF >> PF5) & 1) << 0;
	btn_hists[BTN_DPAD_UP] |= ((PIND >> PD1) & 1) << 0;
	btn_hists[BTN_DPAD_DOWN] |= ((PINB >> PB7) & 1) << 0;
	btn_hists[BTN_DPAD_LEFT] |= ((PINB >> PB1) & 1) << 0;
	btn_hists[BTN_DPAD_RIGHT] |= ((PIND >> PD0) & 1) << 0;

	for (unsigned char i = 0; i < NUM_BUTTONS; i++) {
		//Pressing
		if (btn_states[i] == BTN_STATE_UP && btn_hists[i] == 0xFF) btn_states[i] = BTN_STATE_DOWN;
		//Released
		else if (btn_states[i] == BTN_STATE_DOWN && btn_hists[i] == 0x00) {
			btn_states[i] = BTN_STATE_UP;
			//Only for shooting
			btn_pressed[i] = BTN_PRESSED;
		}
	}
}

ISR(TIMER0_OVF_vect) {
	// Interrupt service routine for TIMER0.
	// TODO
	if (player_bullet_timer)
		player_bullet_timer -= 2;

	for (uint8_t j = 0; j < MAX_BULLET_COUNT_PLAYER; j++)
	{
		if (player_bullet_x_timer[j])
			player_bullet_x_timer[j] -= 2;
	}

	if (alien_timer)
		alien_timer -= 2;

	if (alien_bullet_timer)
		alien_bullet_timer -= 2;

	if (alien_bullet_loop_delay)
		alien_bullet_loop_delay -= 2;
}
