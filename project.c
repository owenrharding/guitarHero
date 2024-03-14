/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by Owen Harding
 */

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

uint16_t game_speed;

/////////////////////////////// main //////////////////////////////////
int main(void)
{
	// Setup hardware and call backs. This will turn on
	// interrupts.
	initialise_hardware();

	// Show the splash screen message. Returns when display
	// is complete.
	start_screen();

	// Loop forever and continuously play the game.
	while (1)
	{
		new_game();
		play_game();
		handle_game_over();
		start_screen();
	}
}

void initialise_hardware(void)
{
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);

	init_timer0();
	init_timer1();
	init_timer2();

	// Turn on global interrupts
	sei();
}

void start_screen(void)
{
	// Clear terminal screen and output a message
	clear_terminal();
	show_cursor();
	clear_terminal();
	hide_cursor();
	set_display_attribute(FG_WHITE);
	move_terminal_cursor(10, 4);
	printf_P(PSTR("  ______   __     __  _______         __    __"));
	move_terminal_cursor(10, 5);
	printf_P(PSTR(" /      \\ |  \\   |  \\|       \\       |  \\  |  \\"));
	move_terminal_cursor(10, 6);
	printf_P(PSTR("|  $$$$$$\\| $$   | $$| $$$$$$$\\      | $$  | $$  ______    ______    ______"));
	move_terminal_cursor(10, 7);
	printf_P(PSTR("| $$__| $$| $$   | $$| $$__| $$      | $$__| $$ /      \\  /      \\  /      \\"));
	move_terminal_cursor(10, 8);
	printf_P(PSTR("| $$    $$ \\$$\\ /  $$| $$    $$      | $$    $$|  $$$$$$\\|  $$$$$$\\|  $$$$$$\\"));
	move_terminal_cursor(10, 9);
	printf_P(PSTR("| $$$$$$$$  \\$$\\  $$ | $$$$$$$\\      | $$$$$$$$| $$    $$| $$   \\$$| $$  | $$"));
	move_terminal_cursor(10, 10);
	printf_P(PSTR("| $$  | $$   \\$$ $$  | $$  | $$      | $$  | $$| $$$$$$$$| $$      | $$__/ $$"));
	move_terminal_cursor(10, 11);
	printf_P(PSTR("| $$  | $$    \\$$$   | $$  | $$      | $$  | $$ \\$$     \\| $$       \\$$    $$"));
	move_terminal_cursor(10, 12);
	printf_P(PSTR(" \\$$   \\$$     \\$     \\$$   \\$$       \\$$   \\$$  \\$$$$$$$ \\$$        \\$$$$$$"));
	move_terminal_cursor(10, 14);
	// change this to your name and student number; remove the chevrons <>
	printf_P(PSTR("CSSE2010/7201 A2 by <OWEN HARDING> - <48007618>"));

	// Output the static start screen and wait for a push button
	// to be pushed or a serial input of 's'
	show_start_screen();

	uint32_t last_screen_update, current_time;
	last_screen_update = get_current_time();

	uint8_t frame_number = 0;
	game_speed = 1000;
	manual_mode = 0;
	game_paused = 0;
	uint8_t manual_mode_printed = 0;

	// Print game speed.
	move_terminal_cursor(10, 16);
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

	// Wait until a button is pressed, or 's' is pressed on the terminal
	while (1)
	{
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S')
		{
			break;
		}
		else if (serial_input == '1')
		{
			game_speed = 1000;
			move_terminal_cursor(22, 16);
			clear_to_end_of_line();
			printf("Normal");
		}
		else if (serial_input == '2')
		{
			game_speed = 500;
			move_terminal_cursor(22, 16);
			clear_to_end_of_line();
			printf("Fast");
		}
		else if (serial_input == '3')
		{
			game_speed = 250;
			move_terminal_cursor(22, 16);
			clear_to_end_of_line();
			printf("Extreme");
		}

		// If serial_input is 'm', then toggle manual_mode.
		else if (serial_input == 'm' || serial_input == 'M')
		{
			manual_mode = !manual_mode;
		}

		if (manual_mode && !manual_mode_printed)
		{
			move_terminal_cursor(10, 18);
			printf_P(PSTR("Manual Mode: ON"));
			manual_mode_printed = !manual_mode_printed;
		}
		else if (!manual_mode && manual_mode_printed)
		{
			move_terminal_cursor(10, 18);
			clear_to_end_of_line();
			manual_mode_printed = !manual_mode_printed;
		}

		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED)
		{
			break;
		}

		// every 200 ms, update the animation
		current_time = get_current_time();
		if (current_time - last_screen_update > game_speed / 5)
		{
			update_start_screen(frame_number);
			frame_number = (frame_number + 1) % 32;
			last_screen_update = current_time;
		}
	}
}

void new_game(void)
{
	uint32_t last_advance_time, current_time;

	// Clear the serial terminal
	clear_terminal();
	print_game_terminal(1);

	uint8_t countdown_nums[4] = {3, 2, 1, 0};
	uint8_t countdown_index = 0;

	last_advance_time = get_current_time();

	while (countdown_index < 5)
	{
		current_time = get_current_time();
		if (countdown_index == 0 ||
			(current_time >= last_advance_time + game_speed))
		{
			// 200ms (0.2 second) has passed since the last time we advance the
			// notes here, so update the advance the notes
			display_countdown(countdown_nums[countdown_index]);
			countdown_index++;
			// Update the most recent time the notes were advance
			last_advance_time = current_time;
		}
	}

	// Initialise the game and display
	initialise_game();

	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void)
{
	uint32_t last_advance_time, current_time;
	int8_t btn; // The button pushed

	last_advance_time = get_current_time();

	// We play the game until it's over
	while (!is_game_over())
	{
		DDRC = 1;
		// We need to check if any button has been pushed, this will be
		// NO_BUTTON_PUSHED if no button has been pushed
		// Checkout the function comment in `buttons.h` and the implementation
		// in `buttons.c`.
		btn = button_pushed();
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}

		if (game_paused)
		{
			move_terminal_cursor(TERMINAL_INDENTATION, GAME_SCORE_ROW - 3);
			printf("Game Paused");
			while (game_paused)
			{
				PORTC = 1 | combo_LEDs;
				if (serial_input_available())
				{
					serial_input = fgetc(stdin);
				}
				if (serial_input == 'p' || serial_input == 'P')
				{
					game_paused = 0;
					serial_input = -1;
				}
			}
			//PORTC = 0 | combo_LEDs;
			move_terminal_cursor(TERMINAL_INDENTATION, GAME_SCORE_ROW - 3);
			clear_to_end_of_line();
		}

		if (serial_input == 'a' || serial_input == 'A')
		{
			btn = 3;
		}
		else if (serial_input == 's' || serial_input == 'S')
		{
			btn = 2;
		}
		else if (serial_input == 'd' || serial_input == 'D')
		{
			btn = 1;
		}
		else if (serial_input == 'f' || serial_input == 'F')
		{
			btn = 0;
		}
		else if (serial_input == 'm' || serial_input == 'M')
		{
			manual_mode = !manual_mode;
			print_game_terminal(1);
		}
		else if (manual_mode && (serial_input == 'n' || serial_input == 'N'))
		{
			advance_note();
		}

		if (btn == BUTTON0_PUSHED)
		{
			// If button 0 play the lowest note (right lane)
			play_note(0);
		}
		else if (btn == BUTTON1_PUSHED)
		{
			// If button 0 play the lowest note (right lane)
			play_note(1);
		}
		else if (btn == BUTTON2_PUSHED)
		{
			// If button 0 play the lowest note (right lane)
			play_note(2);
		}
		else if (btn == BUTTON3_PUSHED)
		{
			// If button 0 play the lowest note (right lane)
			play_note(3);
		}

		if (serial_input == 'p' || serial_input == 'P')
		{
			game_paused = 1;
		}

		// Toggle advance_note control based on manual_mode flag.
		if (!manual_mode)
		{
			current_time = get_current_time();
			if (current_time >= last_advance_time + game_speed / 5)
			{
				// 200ms (0.2 second) has passed since the last time we advance the
				// notes here, so update the advance the notes
				advance_note();

				// Update the most recent time the notes were advance
				last_advance_time = current_time;
			}
		}

		PORTC = 0 | combo_LEDs;
	}
	// We get here if the game is over.
}

void handle_game_over()
{
	move_terminal_cursor(10, 14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10, 15);
	printf_P(PSTR("Press a button or 's'/'S' to start a new game"));

	// Do nothing until a button is pushed. Hint: 's'/'S' should also start a
	// new game
	char serial_input = -1;
	while (button_pushed() == NO_BUTTON_PUSHED && (serial_input != 's') && (serial_input != 'S'))
	{
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}; // wait
	}
}
