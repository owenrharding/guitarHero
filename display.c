/*
 * display.c
 *
 * Authors: Luke Kamols, Jarrod Bennett, Martin Ploschner, Cody Burnett,
 * Renee Nightingale
 */ 

#include "display.h"
#include <stdio.h>
#include <avr/io.h>
#include "pixel_colour.h"
#include "ledmatrix.h"
#include "game.h"

// constant value used to display 'AVR HERO' on launch
static const uint8_t pong_display[MATRIX_NUM_COLUMNS] = 
		{127, 164, 127, 0, 239, 29, 233, 0, 255, 170, 85, 0, 6, 9, 6, 0};

// Fonts for LED Matrix score display
// Stored as a 5 x 3 grid pattern going from Left-to-Right, Top-to-Bottom
// Padded with a leading zero so that it fits into a 16-bit value


void show_start_screen(void)
{
	MatrixColumn column_colour_data;
	uint8_t col_data;
		
	ledmatrix_clear(); // start by clearing the LED matrix
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
	{
		col_data = pong_display[col];
		// go through the top 7 bits (not the bottom one as that was our colour bit)
		// and set any to be the correct colour
		for(uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)
		{
			// If the relevant font bit is set, we make this a coloured pixel, else blank
			if(col_data>>row & 1)
			{
				column_colour_data[row] = (row < 4 ? COLOUR_RED : COLOUR_GREEN);
			}
			else
			{
				column_colour_data[row] = 0;
			}
		}
		ledmatrix_update_column(col, column_colour_data);
	}
	update_start_screen(0);
}

// Update dynamic start screen based on the frame number (0-31)
// Note: this is hardcoded to PONG game.
// Purposefully obfuscated so functionality cannot be copied for movement tasks
void update_start_screen(uint8_t frame_number)
{
	PixelColour colour;
	for (uint8_t row = 4; row < 8; row++)
	{
		for (uint8_t col = 11; col < 16; col++)
		{
			colour = col == 14 ? COLOUR_YELLOW : COLOUR_BLACK;
			if (((32+col-frame_number) & ((1<<(row-2))-1)) == (1<<(row-3))-1)
			{
				colour = col < 14 ? COLOUR_RED : COLOUR_GREEN;
			}
			ledmatrix_update_pixel(col, row, colour);
		}
	}
}

// Initialise the display for the board, this creates the display
// for an empty board.
void default_grid(void)
{
	ledmatrix_clear();
	MatrixColumn colours;
	
	for (uint8_t row=0; row<MATRIX_NUM_ROWS; row++)
	{
		colours[row] = COLOUR_YELLOW;
	}
	ledmatrix_update_column(13, colours);
	
	for (uint8_t row=0; row<MATRIX_NUM_ROWS; row++)
	{
		colours[row] = COLOUR_HALF_YELLOW;
	}
	ledmatrix_update_column(12, colours);
	ledmatrix_update_column(14, colours);
	
	for (uint8_t row=0; row<MATRIX_NUM_ROWS; row++)
	{
		colours[row] = COLOUR_QUART_YELLOW;
	}
	ledmatrix_update_column(11, colours);
	ledmatrix_update_column(15, colours);
}


// constant value used to display '3' on countdown.
static const uint8_t data_3[MATRIX_NUM_COLUMNS] = 
		{0, 0, 0, 0, 60, 102, 96, 28, 96, 102, 60, 0, 0, 0, 0, 0};
// constant value used to display '2' on countdown.
static const uint8_t data_2[MATRIX_NUM_COLUMNS] = 
		{0, 0, 0, 0, 60, 102, 96, 28, 12, 6, 126, 0, 0, 0, 0, 0};
// constant value used to display '1' on countdown.
static const uint8_t data_1[MATRIX_NUM_COLUMNS] = 
		{0, 0, 0, 0, 24, 28, 24, 24, 24, 24, 60, 0, 0, 0, 0, 0};
// constant value used to display 'GO' on countdown.
static const uint8_t data_GO[MATRIX_NUM_COLUMNS] = 
		{0, 0, 0, 0, 70, 161, 161, 165, 165, 165, 70, 0, 0, 0, 0, 0};

// Displays the given number on the LED matrix.
// Arg number: one of {3, 2, 1, 0}.
// Input Displays 'GO'.
void display_countdown(uint8_t number)
{
	uint8_t display_data[MATRIX_NUM_COLUMNS];
	switch (number) {
        case 3:
            for (uint8_t index = 0; index < MATRIX_NUM_COLUMNS; index++) {
                display_data[index] = data_3[index];
            }
            break;
        case 2:
            for (uint8_t index = 0; index < MATRIX_NUM_COLUMNS; index++) {
                display_data[index] = data_2[index];
            }
            break;
        case 1:
            for (uint8_t index = 0; index < MATRIX_NUM_COLUMNS; index++) {
                display_data[index] = data_1[index];
            }
            break;
        case 0:
            for (uint8_t index = 0; index < MATRIX_NUM_COLUMNS; index++) {
                display_data[index] = data_GO[index];
            }
            break;
    }
MatrixColumn column_colour_data;
	uint8_t col_data;
		
	ledmatrix_clear(); // start by clearing the LED matrix
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
	{
		col_data = display_data[col];
		for(uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)
		{
			// If the relevant font bit is set, we make this a coloured pixel, else blank
			if(col_data>>row & 1)
			{
				column_colour_data[row] = (COLOUR_GREEN);
			}
			else
			{
				column_colour_data[row] = 0;
			}
		}
		ledmatrix_update_column(col, column_colour_data);
	}
}