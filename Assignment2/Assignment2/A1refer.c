//
// Insert Assignment 1 here, if your solution fits in a single file.                ^
//                                                                                  |
// Otherwise, leave this box empty and attach up to 19 source files ----------------+
//
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "cab202_graphics.h"
#include "cab202_sprites.h"
#include "cab202_timers.h"
#define N_ALIENS 10                
#define MAX_BULLETS 4

typedef struct Game {
	bool over;
	bool all_die;
	int score;
	int live;
	int event_loop_delay;
	int level;
	int number_of_alive;
	timer_id aliens_bullet_loop_delay;
	double x1, y1, y2, x2, y3, x3, deno, aa, bb, cc;

	sprite_id ship;

	sprite_id bullet;
	timer_id bullet_timer;
	sprite_id game_over_text;

	sprite_id aliens_bullet[MAX_BULLETS];
	timer_id aliens_bullet_timer;

	sprite_id aliens[N_ALIENS];
	timer_id alien_timer;
} Game;
void setup_all(Game * game);
int bullet_counter = 0;
double amplitude = 3;
double period = 8;
int new_x = 100;
int m = 0;
bool check_check = false;
#define KEY_LEFT 'a'
#define KEY_RIGHT 'd'
//-----------------------------------------------------------------------//
/**
*	Gets a random integer that is greater than or equal to
*	first and less than last-1.
*/
double rand_between() {
	double a[] = { -1,1,2 };
	int i = rand() % 3; // here 14 is number of element in array.
	return a[i];

}

int rand_between2() {
	int b[] = { 0,1,2,3,6,7,8,9 };
	int j = rand() % 3; // here 14 is number of element in array.
	return b[j];
}

void setup_ship(Game * game) {
	// TODO
	char* shape_ship[] =
	{
		"$"
	};
	game->ship = create_sprite(screen_width() / 2, screen_height() - 4, 1, 1, *shape_ship);
	game->ship->is_visible = true;
}

void launch_allien_bullet(Game * game) {


	int x = rand() % 10;


	while (!game->aliens[x]->is_visible)
	{
		x = rand() % 10;
	}
	while (x == new_x)
	{
		x = rand() % 10;
	}
	new_x = x;
	for (int i = 0; i<bullet_counter + 1; i++)
	{
		if (!game->aliens_bullet[i]->is_visible)
		{

			game->aliens_bullet[i]->x = game->aliens[x]->x;
			game->aliens_bullet[i]->y = game->aliens[x]->y + 1;
			game->aliens_bullet[i]->is_visible = true;
		}
	}

}
bool move_aliens_bullet(sprite_id a_b, Game * game) {
	if (a_b->is_visible)
	{
		a_b->y = a_b->y + 1;
		if ((int)a_b->y >= screen_height() - 3) //if the coodinate y of alien_bullet > spacecraft then disappear
		{
			a_b->x = -1;
			a_b->y = -1;
			bullet_counter--;
			a_b->is_visible = false;
		}

		if ((int)a_b->x == (int)game->ship->x && (int)a_b->y == (int)game->ship->y)
		{
			a_b->is_visible = false;
			game->live -= 1;
			a_b->x = -1;
			a_b->y = -1;
			bullet_counter--;
			game->ship->x = -1;
			game->ship->y = -1;
			setup_ship(game);
		}

		return true;
	}

	return false;
}
bool update_aliens_bullet(Game * game) {
	bool changed2 = false;
	if (bullet_counter<4)
	{
		if (timer_expired(game->aliens_bullet_loop_delay))
		{
			launch_allien_bullet(game);
			bullet_counter++;
		}
	}

	if (timer_expired(game->aliens_bullet_timer))
	{
		for (int i = 0; i<MAX_BULLETS; i++)
		{
			sprite_id a_b = game->aliens_bullet[i];
			changed2 = move_aliens_bullet(a_b, game);
		}
	}
	return changed2;
}

bool move_alien4(Game *game) {
	int new_x = 0;
	int old_x = 0;
	int new_y = 0;
	int rand_dx[10];
	for (int i = 0; i<10; i++)
	{
		rand_dx[i] = rand_between();
	}
	if ((game->aliens[0]->x + rand_dx[0]) < (game->aliens[1]->x - 1) && (game->aliens[0]->x + rand_dx[0]) > 0)
	{
		game->aliens[0]->x += rand_dx[0];
		game->aliens[0]->y += 1;
	}

	else
	{
		game->aliens[0]->y += 1;
	}

	if ((int)game->aliens[0]->y >= screen_height() - 3)
	{
		game->aliens[0]->y = 0;
	}
	if ((int)game->aliens[0]->x >= screen_width())
	{
		game->aliens[0]->x = 0;
	}


	if ((game->aliens[1]->x + rand_dx[1]) < (game->aliens[2]->x - 1) && (game->aliens[1]->x + rand_dx[1]) > game->aliens[0]->x + 1)
	{
		game->aliens[1]->x += rand_dx[1];
		game->aliens[1]->y += 1;
	}
	else
	{
		game->aliens[1]->y += 1;
	}

	if ((int)game->aliens[1]->y >= screen_height() - 3)
	{
		game->aliens[1]->y = 0;
	}
	if ((int)game->aliens[1]->x >= screen_width())
	{
		game->aliens[1]->x = 0;
	}

	if ((game->aliens[2]->x + rand_dx[2]) > game->aliens[1]->x + 1)
	{
		game->aliens[2]->x += rand_dx[2];
		game->aliens[2]->y += 1;
	}
	else
	{
		game->aliens[2]->y += 1;
	}

	if (game->aliens[2]->y >= screen_height() - 3)
	{
		game->aliens[2]->y = 0;
	}

	if ((int)game->aliens[2]->x >= screen_width())
	{
		game->aliens[2]->x = 0;
	}


	if ((game->aliens[3]->x + rand_dx[3]) < (game->aliens[4]->x - 1) && (game->aliens[3]->x + rand_dx[3]) > 0)
	{
		game->aliens[3]->x += rand_dx[3];
		game->aliens[3]->y += 1;
	}
	else
	{
		game->aliens[3]->y += 1;
	}

	if (game->aliens[3]->y >= screen_height() - 3)
	{
		game->aliens[3]->y = 0;
	}

	if ((int)game->aliens[1]->x >= screen_width())
	{
		game->aliens[3]->x = 0;
	}

	if ((game->aliens[4]->x + rand_dx[4]) < (game->aliens[5]->x - 1) && (game->aliens[4]->x + rand_dx[4]) > game->aliens[3]->x + 1)
	{
		game->aliens[4]->x += rand_dx[4];
		game->aliens[4]->y += 1;
	}
	else
	{
		game->aliens[4]->y += 1;
	}

	if (game->aliens[4]->y >= screen_height() - 3)
	{
		game->aliens[4]->y = 0;
	}
	if ((int)game->aliens[1]->x >= screen_width())
	{
		game->aliens[4]->x = 0;
	}

	if ((game->aliens[5]->x + rand_dx[5]) < (game->aliens[6]->x - 1) && (game->aliens[5]->x + rand_dx[5]) > game->aliens[4]->x + 1)
	{
		game->aliens[5]->x += rand_dx[5];
		game->aliens[5]->y += 1;
	}
	else
	{
		game->aliens[5]->y += 1;
	}

	if (game->aliens[5]->y >= screen_height() - 3)
	{
		game->aliens[5]->y = 0;
	}
	if ((int)game->aliens[5]->x >= screen_width())
	{
		game->aliens[5]->x = 0;
	}
	if ((game->aliens[6]->x + rand_dx[6]) > game->aliens[5]->x + 1)
	{
		game->aliens[6]->x += rand_dx[6];
		game->aliens[6]->y += 1;
	}
	else
	{
		game->aliens[6]->y += 1;
	}

	if (game->aliens[6]->y >= screen_height() - 3)
	{
		game->aliens[6]->y = 0;
	}
	if ((int)game->aliens[6]->x >= screen_width())
	{
		game->aliens[6]->x = 0;
	}
	if ((game->aliens[7]->x + rand_dx[7]) < (game->aliens[8]->x - 1) && (game->aliens[7]->x + rand_dx[7]) > 0)
	{
		game->aliens[7]->x += rand_dx[7];
		game->aliens[7]->y += 1;
	}
	else
	{
		game->aliens[7]->y += 1;
	}

	if (game->aliens[7]->y >= screen_height() - 3)
	{
		game->aliens[7]->y = 0;
	}

	if ((int)game->aliens[7]->x >= screen_width())
	{
		game->aliens[7]->x = 0;
	}

	if ((game->aliens[8]->x + rand_dx[8]) < (game->aliens[9]->x - 1) && (game->aliens[8]->x + rand_dx[8]) > game->aliens[7]->x + 1)
	{
		game->aliens[8]->x += rand_dx[8];
		game->aliens[8]->y += 1;
	}
	else
	{
		game->aliens[8]->y += 1;
	}
	if (game->aliens[8]->y >= screen_height() - 3)
	{
		game->aliens[8]->y = 0;
	}

	if ((int)game->aliens[8]->x >= screen_width())
	{
		game->aliens[8]->x = 0;
	}
	if ((game->aliens[9]->x + rand_dx[9]) > game->aliens[8]->x + 1)
	{
		game->aliens[9]->x += rand_dx[9];
		game->aliens[9]->y += 1;
	}
	else
	{
		game->aliens[9]->y += 1;
	}
	if (game->aliens[9]->y >= screen_height() - 3)
	{
		game->aliens[9]->y = 0;
	}
	if ((int)game->aliens[9]->x >= screen_width())
	{
		game->aliens[9]->x = 0;
	}
	for (int j = 0; j<N_ALIENS; j++)
	{
		new_x = round(game->aliens[j]->x);
		old_x = round(game->aliens[j]->x) - 1;
		new_y = round(game->aliens[j]->y);
		if (new_x == (int)game->ship->x && new_y == (int)game->ship->y)
		{
			game->live -= 1;
			if (round(game->aliens[0]->x) >screen_width() / 2)
			{
				game->ship->x = 0;
			}
			else if (round(game->aliens[0]->x) < screen_width() / 2)
			{
				game->ship->x = screen_width() - 1;
			}
			game->ship->y = screen_height() - 4;
		}

		if (new_x != old_x)
		{
			return true;
		}
	}


	return false;
}

bool move_alien3(sprite_id alien, Game *game) {
	// INSERT CODE HERE

	if (alien->is_visible)
	{
		int old_x = round(alien->x);
		alien->x += 1;
		alien->y += 1;
		int new_x = round(alien->x);
		int new_y = round(alien->y);
		if (new_x == (int)game->ship->x && new_y == (int)game->ship->y)
		{
			game->live -= 1;
			if (round(game->aliens[0]->x) >screen_width() / 2)
			{
				game->ship->x = 0;
			}
			else if (round(game->aliens[0]->x) < screen_width() / 2)
			{
				game->ship->x = screen_width() - 1;
			}
			game->ship->y = screen_height() - 4;
		}
		if (new_x >= screen_width())
		{
			alien->x = 0;
		}

		if (new_y >= screen_height() - 3)
		{
			alien->y = 0;
		}

		if (old_x != new_x)
		{
			return true;
		}
	}
	return false;
}

bool move_alien2(Game *game) {
	int new_x = 0;
	int old_x = 0;
	game->aliens[0]->x++;
	game->aliens[0]->y = amplitude * sin((2 * 3.14) / period*(game->aliens[0]->x)) + 4;
	if (game->aliens[0]->x == screen_width())
	{
		game->aliens[0]->x = 0;
	}


	game->aliens[1]->x++;
	game->aliens[1]->y = game->aliens[0]->y;
	if (game->aliens[1]->x == screen_width())
	{
		game->aliens[1]->x = 0;
	}
	game->aliens[2]->x++;
	game->aliens[2]->y = game->aliens[0]->y;
	if (game->aliens[2]->x == screen_width())
	{
		game->aliens[2]->x = 0;
	}
	game->aliens[3]->x++;
	game->aliens[3]->y = game->aliens[0]->y + 2;
	if (game->aliens[3]->x == screen_width())
	{
		game->aliens[3]->x = 0;
	}

	game->aliens[4]->x++;
	game->aliens[4]->y = game->aliens[0]->y + 2;
	if (game->aliens[4]->x == screen_width())
	{
		game->aliens[4]->x = 0;
	}

	game->aliens[5]->x++;
	game->aliens[5]->y = game->aliens[0]->y + 2;
	if (game->aliens[5]->x == screen_width())
	{
		game->aliens[5]->x = 0;
	}

	game->aliens[6]->x++;
	game->aliens[6]->y = game->aliens[0]->y + 2;
	if (game->aliens[6]->x == screen_width())
	{
		game->aliens[6]->x = 0;
	}

	game->aliens[7]->x++;
	game->aliens[7]->y = game->aliens[0]->y + 4;
	if (game->aliens[7]->x == screen_width())
	{
		game->aliens[7]->x = 0;
	}

	game->aliens[8]->x++;
	game->aliens[8]->y = game->aliens[0]->y + 4;
	if (game->aliens[8]->x == screen_width())
	{
		game->aliens[8]->x = 0;
	}

	game->aliens[9]->x++;
	game->aliens[9]->y = game->aliens[0]->y + 4;
	if (game->aliens[9]->x == screen_width())
	{
		game->aliens[9]->x = 0;
	}

	for (int i = 0; i<N_ALIENS; i++)
	{
		new_x = round(game->aliens[i]->x);
		old_x = round(game->aliens[i]->x) - 1;

		if (new_x != old_x)
		{
			return true;
		}

	}


	return false;
}


bool move_alien5(sprite_id alien, Game * game, int m) {

	if (!check_check)
	{
		check_check = true;
		game->x1 = alien->y;
		game->y1 = alien->x;

		if (game->aliens[4]->x > screen_width() / 2)
		{
			game->y2 = (rand() % 8 + 4);
		}

		else if (game->aliens[4]->x < screen_width() / 2)
		{
			game->y2 = screen_width() - (rand() % 8 + 4);
		}
		game->x2 = (game->ship->y + game->x1) / 2;

		game->y3 = game->ship->x;
		game->x3 = game->ship->y;

		game->deno = (game->x1 - game->x2)*(game->x1 - game->x3)*(game->x2 - game->x3);
		game->aa = ((game->x3*(game->y2 - game->y1)) + (game->x2*(game->y1 - game->y3)) + (game->x1*(game->y3 - game->y2))) / game->deno;
		game->bb = ((game->x3*game->x3*(game->y1 - game->y2)) + (game->x2*game->x2*(game->y3 - game->y1)) + (game->x1*game->x1*(game->y2 - game->y3))) / game->deno;
		game->cc = ((game->x2*game->x3*game->y1*(game->x2 - game->x3)) + (game->x3*game->x1*game->y2*(game->x3 - game->x1)) + (game->x1*game->x2*game->y3*(game->x1 - game->x2))) / game->deno;
	}
	else if (check_check)
	{
		alien->y++;
		alien->x = game->aa * (alien->y * alien->y) + (game->bb*alien->y) + game->cc;
	}
	int new_x = round(alien->x);
	int new_y = round(alien->y);

	if (new_x == (int)game->ship->x && new_y == (int)game->ship->y)
	{
		game->live -= 1;
		if (round(game->aliens[0]->x) >screen_width() / 2)
		{
			game->ship->x = 9;
		}
		else if (round(game->aliens[0]->x) < screen_width() / 2)
		{
			game->ship->x = screen_width() - 10;
		}
		game->ship->y = screen_height() - 4;
	}

	if (new_x > screen_width() - 1 || new_y >= screen_height() - 3)
	{
		if (m == 0 || m == 1 || m == 3 || m == 7 || m == 8)
		{
			alien->x = game->aliens[m + 1]->x - 4;
			alien->y = game->aliens[m + 1]->y;
		}
		else if (m == 2 || m == 6 || m == 9)
		{
			alien->x = game->aliens[m - 1]->x + 4;
			alien->y = game->aliens[m - 1]->y;
		}

		check_check = false;
	}

	return true;
}



bool move_alien1(sprite_id alien) {
	// INSERT CODE HERE
	if (alien->is_visible)
	{
		int old_x = round(alien->x);
		alien->x += 1;
		int new_x = round(alien->x);

		if (new_x >= screen_width())
		{
			alien->x = 0;
		}

		if (old_x != new_x)
		{
			return true;
		}
	}
	return false;
}


bool update_aliens(Game * game) {
	if (!timer_expired(game->alien_timer)) {
		return false;
	}

	bool changed = false;
	if (game->level == 1)
	{
		for (int i = 0; i < N_ALIENS; i++) {
			sprite_id alien = game->aliens[i];
			changed = move_alien1(alien) || changed;

		}
	}

	else if (game->level == 2)
	{
		changed = move_alien2(game) || changed;
	}

	else if (game->level == 3)
	{
		for (int i = 0; i < N_ALIENS; i++) {
			sprite_id alien = game->aliens[i];
			changed = move_alien3(alien, game) || changed;

		}
	}

	else if (game->level == 4)
	{
		changed = move_alien4(game) || changed;
	}

	else if (game->level == 5)
	{
		for (int i = 0; i < N_ALIENS; i++) {
			game->aliens[i]->x++;
			game->aliens[i]->y++;
			if (round(game->aliens[i]->x) >= screen_width())
			{
				game->aliens[i]->x = 0;
			}
			if (round(game->aliens[i]->y) >= screen_height() - 3)
			{
				game->aliens[i]->y = 0;
			}
		}
		if (!check_check)
		{
			m = rand_between2();

			while (!game->aliens[m]->is_visible)
			{
				m = rand_between2();
			}
			if (m == 0 || m == 1 || m == 2)
			{
				game->aliens[m]->y -= 2;
			}
			else if (m == 7 || m == 8 || m == 9)
			{
				game->aliens[m]->y += 2;
			}
			else if (m == 3)
			{
				game->aliens[m]->x -= 2;
			}
			else if (m == 6)
			{
				game->aliens[m]->x += 2;
			}
		}
		sprite_id alien = game->aliens[m];
		changed = move_alien5(alien, game, m) || changed;
	}
	return changed;
}
bool move_bullet(Game * game) {
	if (game->bullet->is_visible)
	{
		if (timer_expired(game->bullet_timer))
		{
			game->bullet->y--;
			int b_y = (int)game->bullet->y;

			if (b_y  < -1)
			{
				game->bullet->is_visible = false;
			}

			for (int i = 0; i< N_ALIENS; i++)
			{
				if ((int)game->bullet->x == (int)game->aliens[i]->x && (int)game->bullet->y == (int)game->aliens[i]->y)
				{
					game->bullet->is_visible = false;
					game->aliens[i]->is_visible = false;
					game->aliens[i]->x = -1;
					game->aliens[i]->y = -1;
					game->score += 30;
				}

			}

			return true;
		}
	}
	return false;
}
void launch_bullet(Game * game) {
	if (!game->bullet->is_visible)
	{
		game->bullet->x = game->ship->x + game->ship->width / 2;
		game->bullet->y = game->ship->y;
		game->bullet->dy = -1;
		game->bullet->dx = 0;
		game->bullet->is_visible = true;
		reset_timer(game->bullet_timer);
	}

}
bool update_bullet(Game * game, int key_code) {
	if (game->bullet->is_visible) {
		return move_bullet(game);
	}
	else if (key_code == 's') {
		launch_bullet(game);
		return true;
	}
	else {
		return false;
	}
}
bool update_ship(Game * game, int key_code) {
	if (key_code == KEY_RIGHT)
	{
		if (game->ship->x < screen_width() - 1)
			game->ship->x++;
		return true;
	}

	else if (key_code == KEY_LEFT)
	{
		if (game->ship->x >= 1)
		{
			game->ship->x--;
		}
		return true;
	}
	return false;
}
void draw_aliens_bullet(Game * game) {
	for (int i = 0; i<MAX_BULLETS; i++) {

		draw_sprite(game->aliens_bullet[i]);
	}
}
void draw_aliens(Game * game) {
	for (int i = 0; i<N_ALIENS; i++) {

		draw_sprite(game->aliens[i]);
	}
}
void draw_bullet(Game * game) {
	draw_sprite(game->bullet);
}
void draw_ship(Game *game) {
	draw_sprite(game->ship);
}
void draw_UI(Game * game) {
	// TODO
	draw_line(0, screen_height() - 3, screen_width() - 1, screen_height() - 3, '_');
	draw_string(0, screen_height() - 2, "Lu Chen Hua (n9543503)");
	draw_formatted(0, screen_height() - 1, "Level: %d", game->level);
	draw_formatted(screen_width() - 11, screen_height() - 2, "Score: %d", game->score);
	draw_formatted(screen_width() - 11, screen_height() - 1, "Live : %d", game->live);
	show_screen();
}
void draw_all(Game * game) {
	clear_screen();
	draw_UI(game);
	draw_ship(game);
	draw_bullet(game);
	draw_aliens(game);
	draw_aliens_bullet(game);
	show_screen();
}
void setup_gameover_text(Game* game)
{
	static char shape[] = {
		"**************************"
		"*                        *"
		"*       GAME  OVER       *"
		"*                        *"
		"*Do you want to restart ?*"
		"*Press R to restart      *"
		"*Press Q to quit         *"
		"**************************"
	};

	game->game_over_text = create_sprite(-100, -100, 26, 8, shape);
	game->game_over_text->is_visible = false;
}
void setup_aliens_bullet(Game * game) {
	// TODO
	static char bitmap[] = { '*' };
	for (int i = 0; i<MAX_BULLETS; i++)
	{
		game->aliens_bullet[i] = create_sprite(-1, -1, 1, 1, bitmap);
		game->aliens_bullet[i]->is_visible = false;
	}
	game->aliens_bullet_timer = create_timer(400);
}
void setup_aliens(Game * game) {
	// TODO
	int f_x = 2;
	int f_y = 3;
	for (int i = 0; i<3; i++)
	{
		game->aliens[i] = create_sprite(f_x, f_y, 1, 1, "@");
		game->aliens[i]->is_visible = true;
		f_x += 4;

	}
	f_x = 0;
	f_y = 5;
	for (int i = 3; i<7; i++)
	{
		game->aliens[i] = create_sprite(f_x, f_y, 1, 1, "@");
		game->aliens[i]->is_visible = true;

		f_x += 4;

	}

	f_x = 2;
	f_y = 7;
	for (int i = 7; i<10; i++)
	{
		game->aliens[i] = create_sprite(f_x, f_y, 1, 1, "@");
		game->aliens[i]->is_visible = true;
		f_x += 4;

	}
}
void setup_bullet(Game * game) {
	// TODO
	static char bitmap[] = { '|' };
	game->bullet = create_sprite(-1, -1, 1, 1, bitmap);
	game->bullet->is_visible = false;
	game->bullet_timer = create_timer(30);
}
void setup_game(Game * game) {
	// INSERT CODE HERE
	game->live = 3;
	game->score = 0;
	game->over = false;
	game->level = 1;
	game->event_loop_delay = 10;
	game->aliens_bullet_loop_delay = create_timer(3000);
	game->alien_timer = create_timer(600);
}
void setup_all(Game * game) {

	setup_gameover_text(game);
	setup_game(game);
	setup_ship(game);
	setup_bullet(game);
	setup_aliens(game);
	setup_aliens_bullet(game);
}
void event_loop(void) {
	Game game;

	setup_all(&game);
	draw_all(&game);

	while (!game.over) {
		int key_code = get_char();

		if (key_code == 'l')
		{
			game.level++;
			setup_aliens(&game);
			if (game.level>5)
			{
				game.level = 1;
			}
		}
		if (key_code == 'q') {
			game.over = true;
		}

		if (key_code == 'r') {
			setup_all(&game);
		}

		else {
			bool show_ship = update_ship(&game, key_code);
			bool show_bullet = update_bullet(&game, key_code);
			bool show_aliens = update_aliens(&game);
			bool show_aliens_bullet = update_aliens_bullet(&game);

			if (show_ship || show_bullet || show_aliens || show_aliens_bullet) {
				if (game.live>0)
				{
					draw_all(&game);
				}

				else
				{
					clear_screen();
					game.game_over_text->is_visible = true;
					game.game_over_text->x = (screen_width() - 28) / 2;
					game.game_over_text->y = (screen_height() - 8) / 2;
					draw_sprite(game.game_over_text);
					show_screen();
				}
			}
			for (int i = 0; i< N_ALIENS; i++)
			{
				if (game.aliens[i]->is_visible == true)
				{
					game.all_die = false;
					break;
				}
				else game.all_die = true;
			}
			if (game.all_die) {
				game.score += 500;
				setup_aliens(&game);
			}
		}
		timer_pause(game.event_loop_delay);
	}

}
int main(void) {
	// Seed the random number generator - based off the system clock so it's different every time
	time_t t;
	srand((unsigned)time(&t));
	srand(time(NULL));
	setup_screen();
	event_loop();
	cleanup_screen();
	return 0;
}