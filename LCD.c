#include "stm32f30x_conf.h"
#include "LCD.h"
#include <stdlib.h>

long X_MAX = 84;
long Y_MAX = 6;

uint8_t LCD_RAM [6][84] = {{0}}; // 0 initialises LCD display array

void init_LCD(void) {
	// Reset Pulse , active low to reset RAM contents, Datasheet p. 15
	GPIO_ResetBits(GPIOB, GPIO_Pin_6);
	
	// Wait for Display to reset
	TIM_Cmd(TIM3,ENABLE);
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);
	while(TIM_GetFlagStatus(TIM3, TIM_FLAG_Update) == 0)
	TIM_Cmd(TIM3,DISABLE);
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_6); //Reset Pin High
	
	//SPI config for LCD, Ref.Man p. 1012, Angabe
	SPI_InitTypeDef SPIT_INIT;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	
	SPIT_INIT.SPI_Mode=SPI_Mode_Master; 						//Master mode 
	SPIT_INIT.SPI_Direction=SPI_Direction_1Line_Tx; // Transmit only
	SPIT_INIT.SPI_DataSize= SPI_DataSize_8b;				// 8 bit dataword
	SPIT_INIT.SPI_BaudRatePrescaler= SPI_BaudRatePrescaler_64; // Baudrate 1Mhz
	SPIT_INIT.SPI_FirstBit=SPI_FirstBit_MSB; 				//MSB First
	SPIT_INIT.SPI_NSS= SPI_NSS_Soft; 								// Slave Select soft mode
	SPIT_INIT.SPI_CPHA=SPI_CPHA_1Edge ; 						// start with first rising edge
	SPIT_INIT.SPI_CPOL=SPI_CPOL_Low ; 							// Low when Ruhe
	
	SPI_Init(SPI1,&SPIT_INIT);
	SPI_Cmd(SPI1, ENABLE);

	// Wait for ongoing transaction to finish
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
	GPIO_ResetBits(GPIOC, GPIO_Pin_7); // set SCE low , datasheet p. 22
	
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
	GPIO_ResetBits(GPIOA, GPIO_Pin_6); //DC low for config information, when D/C == 1 data transfer

	// LCD INIT on Datasheet, p.22
	dataSend(0x20);  // p. 22, row 4	0b00100000
	dataSend(0x0C);  // p. 22, row 5	0b00001100
	
	clearDisplay();
}

void drawGame(field_t field) {
	clearDisplay();
	drawBall(field.ball);
	drawBar(field.paddle);
}

void drawBall(ball_t ball) {
		if (ball.x >= 0 && ball.x <= X_MAX-2) { // -2 because of width
			if (ball.y >= 0 && ball.y < Y_MAX) {
				LCD_RAM[ball.y][ball.x] = 0x3 << (ball.height*2); // 0x7e gives a 01111110 signal for the vertical part 
				LCD_RAM[ball.y][ball.x+1] = 0x3 << (ball.height*2);
			}
		}
	
	refreshDisplay();
}

void drawBar(paddle_t paddle) {	
	if (paddle.x >= 0 && paddle.x <= X_MAX-2) {
		if (paddle.y >= 0 && paddle.y < Y_MAX) {
			LCD_RAM[paddle.y][paddle.x] = 0xFF << (paddle.height*2); 
			LCD_RAM[paddle.y][paddle.x+1] = 0xFF << (paddle.height*2);
			
			uint8_t shift_value = 0;
			for (int i = 0; i < paddle.height+1; i++) {
				shift_value |= 0x3 << (i*2);
			}

			LCD_RAM[paddle.y+1][paddle.x] = shift_value;
			LCD_RAM[paddle.y+1][paddle.x+1] = shift_value;
		}
	}
	
	refreshDisplay();
}


void clearDisplay(){
	for(int y = 0; y < Y_MAX; y++) {		
		for(int x = 0; x < X_MAX; x++) {
			LCD_RAM[y][x] = 0;
		}
	}	
	
	refreshDisplay();
}
	
void dataSend(uint8_t data){
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET); 
	SPI_SendData8(SPI1, data);
}


void refreshDisplay(){	
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)); 
	GPIO_SetBits(GPIOA,GPIO_Pin_6);
	
	for(int y = 0; y < Y_MAX; y++) {
		for(int x = 0; x < X_MAX; x++) {
				dataSend(LCD_RAM[y][x]);
		}
	}
}


field_t game_logic(field_t field, int input_up) {
	// Calculates Next state of the game, given the current field
	// input_up == 1, when plate tilted up
	paddle_t bar = field.paddle;
	ball_t ball = field.ball;

	// Ball Collision with paddle
	if (field.ball.x + ball.size + ball.dx >= bar.x) { // Collision horizontal
		// Calculate position in absolute bits: y [0, Y_MAX*8], 
		int real_y_ball = ball.y * Y_MAX + ball.height; 
		
		if (real_y_ball > field.paddle.y*Y_MAX && real_y_ball < field.paddle.y*Y_MAX + 10)  {
			field.ball.dx *= -1;
		}
	}
		
	// Game over?
	if (field.ball.x + ball.size + ball.dx >= X_MAX) { // Collision horizontal
		return initGame();
	}
	
	// Ball Collision with wall
	if (field.ball.x + field.ball.dx < 1) {
		field.ball.dx = abs(field.ball.dx);
	}
	if (field.ball.y == 0 && field.ball.height == 0) {
		field.ball.dy = abs(field.ball.dy);
	}
	if (field.ball.y == 5 && field.ball.height == 3) {
		field.ball.dy = -abs(field.ball.dy);
	}
	
	// Paddle Collision with wall
	if (field.paddle.y == 0 && field.paddle.height == 0 && input_up < 0) {
		input_up = 0;
	}
	else if (field.paddle.y == 5 - 1 && field.paddle.height == 3 && input_up > 0) {
		input_up = 0;
	}
	
	// Update Ball Position
	field.ball.x += ball.dx;
	
	if (field.ball.dy > 0) {
		field.ball.height += 1;
		if (field.ball.height > 3) {
			field.ball.height = 0;
			field.ball.y += field.ball.dy;
		}
	}
	else if (field.ball.dy < 0) {
		field.ball.height -= 1;
		if (field.ball.height < 0) {
			field.ball.height = 3;
			field.ball.y += field.ball.dy;
		}
	}
	
	// Update Bar Position
	if (input_up > 0) {
		field.paddle.height += 1;
		if (field.paddle.height > 3) {
			field.paddle.height = 0;
			field.paddle.y += input_up;
		}
	}
	else if (input_up < 0) {
		field.paddle.height -= 1;
		if (field.paddle.height < 0) {
			field.paddle.height = 3;
			field.paddle.y += input_up;
		}
	}
	
	return field;
}

field_t initGame(void) {
	ball_t ball;
	ball.x = X_MAX / 2;
	ball.y = Y_MAX / 2 - 1;
	ball.dx = 2;
	ball.dy = 1;
	ball.size = 2;
	ball.height = 2;
	
	paddle_t paddle;
	paddle.x = X_MAX - 2;
	paddle.y = Y_MAX / 2 - 1;
	paddle.height = 1;
	
	field_t field;
	field.ball = ball;
	field.paddle = paddle;
	
	return field;
}
