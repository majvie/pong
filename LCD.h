#include "stm32f30x_conf.h"

typedef struct paddle_s {
  int x;	// Position from top left point
	int y;
	int width;
	int height; //// 0->3, gives height of ball in yfield
} paddle_t;

typedef struct ball_s {
	int x;
	int y;
	int dx;
	int dy;
	int size;
	int height; // 0->3, gives height of ball in yfield
} ball_t;

typedef struct field_s {
	paddle_t paddle;
	ball_t	ball;
} field_t;


void init_LCD(void);

void clearDisplay(void);

void dataSend(uint8_t data);

void refreshDisplay(void);

void drawBall(ball_t game_ball);

void drawBar(paddle_t player_bar);

field_t game_logic(field_t playing_field, int input_up);

field_t initGame(void);

void drawGame(field_t playing_field);
