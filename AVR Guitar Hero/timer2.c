/*
 * timer2.c
 *
 * Author: Peter Sutton
 *
 * timer 2 skeleton
 */

#include "timer2.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "game.h"

/* Seven segment display values */
uint8_t seven_seg_data[11] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111, 64};

volatile uint8_t stopwatch_timing = 0;

/* digits_displayed - 1 if digits are displayed on the seven
** segment display, 0 if not. No digits displayed initially.
*/
volatile uint8_t digits_displayed = 0;

/* Time value - we count hundredths of seconds,
** i.e. increment the count every 10ms.
*/
volatile uint16_t count = 0;

/* Seven segment display digit being displayed.
** 0 = right digit; 1 = left digit.
*/
volatile uint8_t seven_seg_cc = 0;

/* Set up timer 2
 */
void init_timer2(void)
{
	DDRA = 0xff;

	TCNT2 = 0;
	/* Set up timer/counter 1 so that we get an
	** interrupt 100 times per second, i.e. every
	** 10 milliseconds.
	*/
	OCR2A = 9999;						 /* Clock divided by 8 - count for 10000 cycles */
	TCCR2A = 0;							 /* CTC mode */
	TCCR2B = (1 << WGM22) | (1 << CS21); /* Divide clock by 8 */

	/* Enable interrupt on timer on output compare match
	 */
	TIMSK2 = (1 << OCIE2A);

	/* Ensure interrupt flag is cleared */
	TIFR2 = (1 << OCF2A);

	stopwatch_timing ^= 1;
	digits_displayed = 1;
}

ISR(TIMER2_COMPA_vect)
{
	uint8_t ones_digit, tens_digit;
	/* If the stopwatch is running then increment time.
	** If we've reached 1000, then wrap this around to 0.
	*/
	if (stopwatch_timing)
	{
		count++;
		if (count == 1000)
		{
			count = 0;
		}
	}

	// Calculate the ones and tens digits
	if (game_score < -9)
	{
		ones_digit = 10; // Display '-' in the ones digit
		tens_digit = 10; // Display '-' in the tens digit
	}
	else if (game_score < 0)
	{
		ones_digit = -(game_score % 10);
		tens_digit = 10; // Display '-' in the tens digit
	}
	else if (game_score < 100)
	{
		ones_digit = game_score % 10;
		tens_digit = (game_score / 10) % 10;
	}
	else
	{
		ones_digit = 0;
		tens_digit = 0;
	}

	// Toggle the seven-segment display digit
	seven_seg_cc ^= 1;

	if (digits_displayed)
	{
		if (seven_seg_cc == 0)
		{
			// Display the rightmost digit (tenths of seconds)
			PORTA = seven_seg_data[ones_digit] | (seven_seg_cc ? 0x80 : 0);
		}
		else
		{
			// Display the leftmost digit (seconds + decimal point)
			PORTA = tens_digit ? (seven_seg_data[tens_digit] | (seven_seg_cc ? 0x80 : 0)) : (seven_seg_cc ? 0x80 : 0);
		}
	}
	else
	{
		// No digits displayed, display is blank
		PORTA = 0;
	}
}