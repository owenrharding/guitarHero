/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */

#include <avr/pgmspace.h>

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "ledmatrix.h"
#include "terminalio.h"
#include "timer2.h"
#include "timer0.h"
#include "timer1.h"

static const uint8_t track[TRACK_LENGTH] = {0x00,
											0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,
											0x04, 0x40, 0x08, 0x80, 0x00, 0x00, 0x04, 0x02,
											0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
											0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x02, 0x20,
											0x04, 0x40, 0x08, 0x80, 0x04, 0x40, 0x02, 0x20,
											0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
											0x04, 0x40, 0x08, 0x04, 0x40, 0x02, 0x20, 0x01,
											0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
											0x00, 0x00, 0x08, 0x08, 0x08, 0x80, 0x04, 0x02,
											0x04, 0x40, 0x02, 0x08, 0x80, 0x00, 0x02, 0x01,
											0x04, 0x40, 0x08, 0x80, 0x04, 0x02, 0x20, 0x01,
											0x10, 0x10, 0x12, 0x20, 0x00, 0x00, 0x02, 0x20,
											0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
											0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
											0x04, 0x40, 0x08, 0x04, 0x40, 0x40, 0x02, 0x20,
											0x01, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};

uint16_t beat;
uint8_t note_mask;
uint8_t long_note;

uint8_t note_hit_successfully;
uint8_t game_speed_printed;

// Initialise the game by resetting the grid and beat
void initialise_game(void)
{
	// initialise the display we are using.
	default_grid();
	game_score = 0;
	combo_count = 0;
	combo_LEDs = 0;
	duty_percentage = 0;

	beat = 0;

	note_mask = 0;
	btn_pressed_during_this_beat = 0;
	game_speed_printed = 0;
	print_game_terminal(1);
}

// Play a note in the given lane
void play_note(uint8_t lane)
{
	// Lane: unsigned integer, one of: {0, 1, 2, 3}. Indicates the btn pushed.

	// Note mask: 8>>lane becomes one of: {1000, 0100, 0010, 0001}.
	lane = (3 - lane) % 4;

	if (!btn_pressed_during_this_beat)
	{
		note_mask = 1 << lane;
	}

	if (btn_pressed_during_this_beat)
	{
		update_game_score(-1, 0);
	}

	// If applicable, turn matrix LEDs green immediately upon function call.
	// This way there isn't delay until next beat for note to turn green.
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
	{
		// col counts from one end, future from the other
		uint8_t future = MATRIX_NUM_COLUMNS - 1 - col;
		uint8_t index = (future + beat) / 5;
		uint8_t note = track[index];
		uint8_t long_note = 0;
		// Filter out short notes by bitwise anding 1111 on left nybble.
		if (note & 0xf0)
		{
			note = note >> 4;
			long_note = 1;
		}
		if ((future + beat) % 5 && long_note == 0)
		{
			continue;
			// notes are only drawn every five columns
		}

		// if the index is beyond the end of the track,
		// no note can be drawn
		if (index >= TRACK_LENGTH)
		{
			continue;
		}

		// iterate over the four paths

		if (note_mask & note & (1 << lane) && (future < 5) && !btn_pressed_during_this_beat && !long_note)
		{
			ledmatrix_update_pixel(col, 2 * lane, COLOUR_GREEN);
			ledmatrix_update_pixel(col, 2 * lane + 1, COLOUR_GREEN);

			if (future == 0 || future == 4)
			{
				update_game_score(1, 0);
				if (future == 4)
				{
					duty_percentage = 2;
				}
				else if (future == 0)
				{
					duty_percentage = 98;
				}
			}
			else if (future == 1 || future == 3)
			{
				update_game_score(2, 0);
				if (future == 3)
				{
					duty_percentage = 10;
				}
				else if (future == 1)
				{
					duty_percentage = 90;
				}
			}
			else if (future == 2)
			{
				if (combo_count > 3)
				{
					update_game_score(4, 1);
				}
				else
				{
					update_game_score(3, 1);
				}
				duty_percentage = 50;
			}

			switch (lane)
			{
				case 0:
					freq = 523.2511;
				case 1:
					freq = 622.2540;
				case 2:
					freq = 698.4565;
				case 3:
					freq = 783.9909;
			}

			note_hit_successfully = 1;
		}
	}

	for (uint16_t future = 16;; future++)
	{
		if ((future + beat) % 5)
		{
			continue;
		}
		uint8_t index = (future + beat) / 5;

		if (index >= TRACK_LENGTH)
		{
			break;
		}
		if (track[index] & 0x0f)
		{
			{
				if (track[index] & (1 << lane) && (combo_count >= 3))
				{
					ledmatrix_update_pixel(0, 2 * lane, COLOUR_QUART_ORANGE);
					ledmatrix_update_pixel(0, 2 * lane + 1, COLOUR_QUART_ORANGE);
				}
				else if (track[index] & (1 << lane))
				{
					ledmatrix_update_pixel(0, 2 * lane, COLOUR_QUART_RED);
					ledmatrix_update_pixel(0, 2 * lane + 1, COLOUR_QUART_RED);
				}
			}
			break;
		}
	}

	// Deduct 1 point if button is pressed without a valid note.
	if (!note_hit_successfully)
	{
		update_game_score(-1, 0);
	}

	btn_pressed_during_this_beat = 1;

	redraw_notes();

	// printf("\rGame Score: %5d", game_score);
	print_game_terminal(0);
}

// Advance the notes one row down the display
void advance_note(void)
{
	// remove all the current notes; reverse of below
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
	{
		uint8_t future = MATRIX_NUM_COLUMNS - 1 - col;
		uint8_t index = (future + beat) / 5;
		uint8_t long_note = 0;
		uint8_t note = track[index];
		if (index >= TRACK_LENGTH)
		{
			break;
		}
		if (note & 0xf0)
		{
			// Shift left nybble into part that gets read.
			note = note >> 4;
		}
		if (track[index + 1] & 0xF0)
		{
			long_note = 1;
		}

		if ((future + beat) % 5 && long_note == 0)
		{
			continue;
			// notes are only drawn every five columns
		}

		for (uint8_t lane = 0; lane < 4; lane++)
		{
			if (note & (1 << lane))
			{
				PixelColour colour;
				// yellows in the scoring area
				if (col == 11 || col == 15)
				{
					colour = COLOUR_QUART_YELLOW;
				}
				else if (col == 12 || col == 14)
				{
					colour = COLOUR_HALF_YELLOW;
				}
				else if (col == 13)
				{
					colour = COLOUR_YELLOW;
				}
				else
				{
					colour = COLOUR_BLACK;
				}
				ledmatrix_update_pixel(col, 2 * lane, colour);
				ledmatrix_update_pixel(col, 2 * lane + 1, colour);
			}
		}
	}

	if (beat % 5 == 0)
	{
		// If a note isn't zero, leaves the board, and didn't get hit, deduct.
		if ((track[(beat / 5)] & 0x0F) && !note_hit_successfully)
		{
			update_game_score(-1, 0);
			print_game_terminal(0);
		}

		note_hit_successfully = 0;
		note_mask = 0;
		btn_pressed_during_this_beat = 0;
	}

	// increment the beat
	beat++;

	for (uint16_t future = 16;; future++)
	{
		if ((future + beat) % 5)
		{
			continue;
		}
		uint8_t index = (future + beat) / 5;

		if (index >= TRACK_LENGTH)
		{
			break;
		}
		if (track[index] & 0x0f)
		{
			for (uint8_t lane = 0; lane < 4; lane++)
			{
				if (track[index] & (1 << lane) && (combo_count >= 3))
				{
					ledmatrix_update_pixel(0, 2 * lane, COLOUR_QUART_ORANGE);
					ledmatrix_update_pixel(0, 2 * lane + 1, COLOUR_QUART_ORANGE);
				}
				else if (track[index] & (1 << lane))
				{
					ledmatrix_update_pixel(0, 2 * lane, COLOUR_QUART_RED);
					ledmatrix_update_pixel(0, 2 * lane + 1, COLOUR_QUART_RED);
				}
			}
			break;
		}
	}

	// draw the new notes
	redraw_notes();
}

void print_game_terminal(uint8_t update_manual_mode)
{

	move_terminal_cursor(TERMINAL_INDENTATION, GAME_SCORE_ROW + 1);
	printf("Combo Count: %4d", combo_count);
	set_display_attribute(FG_WHITE);
	move_terminal_cursor(TERMINAL_INDENTATION, GAME_SCORE_ROW);
	printf("Game Score: %5d", game_score);

	move_terminal_cursor(TERMINAL_INDENTATION, SETTINGS_BAR_ROW);
	printf("SETTINGS");
	if (update_manual_mode)
	{
		move_terminal_cursor(TERMINAL_INDENTATION, SETTINGS_BAR_ROW + 1);
		clear_to_end_of_line();
		if (manual_mode)
		{
			printf("Manual Mode:   ON");
		}
		else
		{
			printf("Manual Mode:  OFF");
		}
	}
	if (!game_speed_printed)
	{
		move_terminal_cursor(TERMINAL_INDENTATION, SETTINGS_BAR_ROW + 2);
		printf("Game Speed: ");
		if (game_speed == 1000)
		{
			printf("Normal");
		}
		else if (game_speed == 500)
		{
			printf("Fast");
		}
		else if (game_speed == 250)
		{
			printf("Extreme");
		}
		game_speed_printed = 1;
	}
	if (combo_count >= 3)
	{
		print_combo();
	}
	else
	{
		clear_combo();
	}
}

void print_combo(void)
{
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW);
	printf_P(PSTR("  ______                           __                  __ "));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 1);
	printf_P(PSTR(" /      \\                         |  \\                |  \\"));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 2);
	printf_P(PSTR("|  $$$$$$\\  ______   ______ ____  | $$____    ______  | $$"));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 3);
	printf_P(PSTR("| $$   \\$$ /      \\ |      \\    \\ | $$    \\  /      \\ | $$"));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 4);
	printf_P(PSTR("| $$      |  $$$$$$\\| $$$$$$\\$$$$\\| $$$$$$$\\|  $$$$$$\\| $$"));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 5);
	printf_P(PSTR("| $$   __ | $$  | $$| $$ | $$ | $$| $$  | $$| $$  | $$ \\$$"));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 5);
	printf_P(PSTR("| $$   __ | $$  | $$| $$ | $$ | $$| $$  | $$| $$  | $$ \\$$"));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 6);
	printf_P(PSTR("| $$__/  \\| $$__/ $$| $$ | $$ | $$| $$__/ $$| $$__/ $$ __ "));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 7);
	printf_P(PSTR(" \\$$    $$ \\$$    $$| $$ | $$ | $$| $$    $$ \\$$    $$|  \\"));
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 8);
	printf_P(PSTR("  \\$$$$$$   \\$$$$$$  \\$$  \\$$  \\$$ \\$$$$$$$   \\$$$$$$  \\$$"));
}

void clear_combo(void)
{
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 1);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 2);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 3);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 4);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 5);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 5);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 6);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 7);
	clear_to_end_of_line();
	move_terminal_cursor(TERMINAL_INDENTATION, COMBO_ROW + 8);
	clear_to_end_of_line();
}

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void)
{
	// YOUR CODE HERE
	if (beat / 5 == TRACK_LENGTH)
	{
		clear_terminal();
		return 1;
	}

	// Detect if the game is over i.e. if a player has won.
	return 0;
}

// Updates game_score and combo_count based on input.
void update_game_score(int update_amount, uint8_t combo)
{
	DDRC = 14;
	game_score += update_amount;
	if (combo)
	{
		combo_count++;
	}
	else
	{
		combo_count = 0;
	}

	if (combo_count == 1)
	{
		combo_LEDs = (1<<1);
	}
	else if (combo_count == 2)
	{
		combo_LEDs = (1<<1) | (1<<2);
	}
	else if (combo_count >= 3)
	{
		combo_LEDs = (1<<1) | (1<<2) | (1<<3);
	}
	else
	{
		combo_LEDs = 0;
	}
	printf("Combo LEDs: %d", combo_LEDs);
}

void redraw_notes(void)
{
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
	{
		// col counts from one end, future from the other
		uint8_t future = MATRIX_NUM_COLUMNS - 1 - col;
		uint8_t index = (future + beat) / 5;
		uint8_t note = track[index];
		uint8_t long_note = 0;
		// Filter out short notes by bitwise anding 1111 on left nybble.
		if (note & 0xf0)
		{
			note = note >> 4;
		}
		if (track[index + 1] & 0xF0)
		{
			long_note = 1;
		}
		if ((future + beat) % 5 && long_note == 0)
		{
			continue;
			// notes are only drawn every five columns
		}

		// if the index is beyond the end of the track,
		// no note can be drawn
		if (index >= TRACK_LENGTH)
		{
			continue;
		}

		uint8_t color = COLOUR_RED;
		if (combo_count >= 3)
		{
			color = COLOUR_ORANGE;
		}
		// iterate over the four paths
		for (uint8_t lane = 0; lane < 4; lane++)
		{
			if (note_mask & note && future < 5)
			{

				color = COLOUR_GREEN;
			}
			// check if there's a note in the specific path
			if (note & (1 << lane))
			{
				// if so, colour the two pixels red
				ledmatrix_update_pixel(col, 2 * lane, color);
				ledmatrix_update_pixel(col, 2 * lane + 1, color);
			}
		}
	}
}