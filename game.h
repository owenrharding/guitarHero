/*
 * game.h
 *
 * Author: Jarrod Bennett, Cody Burnett
 *
 * Function prototypes for game functions available externally. You may wish
 * to add extra function prototypes here to make other functions available to
 * other files.
 */


#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>

#define TRACK_LENGTH 129

#define TERMINAL_INDENTATION 10
#define SETTINGS_BAR_ROW 12
#define GAME_SCORE_ROW 8
#define COMBO_ROW 16

uint8_t manual_mode;
uint16_t game_speed;
int16_t game_score;
uint8_t combo_count;
uint16_t duty_percentage;
uint8_t combo_LEDs;
volatile uint8_t btn_pressed_during_this_beat;

// Initialise the game by resetting the grid and beat
void initialise_game(void);

// Play a note in the given lane
void play_note(uint8_t lane);

// Advance the notes one row down the display
void advance_note(void);

// Prints game terminal information.
void print_game_terminal(uint8_t update_manual_mode);

// Redraws notes on the LED matrix.
void redraw_notes(void);

// Updates game_score variable as well as handling SSD functionality.
void update_game_score(int update_amount, uint8_t combo);

// Prints ASCII 'Combo!' to the terminal.
void print_combo(void);

// Clears ASCII 'Combo!' from the terminal.
void clear_combo(void);

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void);

#endif
