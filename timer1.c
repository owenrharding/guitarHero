/*
 * timer1.c
 *
 * Author: Peter Sutton, Owen Harding
 *
 * timer 1 skeleton
 */

#include "timer1.h"
#include "game.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// For a given frequency (Hz), return the clock period (in terms of the
// number of clock cycles of a 1MHz clock)
uint16_t freq_to_clock_period(uint16_t freq)
{
	return (1000000UL / freq); // UL makes the constant an unsigned long (32 bits)
							   // and ensures we do 32 bit arithmetic, not 16
}

// Return the width of a pulse (in clock cycles) given a duty cycle (%) and
// the period of the clock (measured in clock cycles)
uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clockperiod)
{
	return (dutycycle * clockperiod) / 100;
}

uint16_t freq = 200; // Hz
float dutycycle = 2; // %
uint16_t clockperiod = 0;
uint16_t pulsewidth = 0;

/* Set up timer 1
 */
void init_timer1(void)
{
	TCNT1 = 0;

	TIMSK1 |= (1 << OCIE1A);

	// Set up timer/counter 1 for Fast PWM, counting from 0 to the value in OCR1A
	// before reseting to 0. Count at 1MHz (CLK/8).
	// Configure output OC1B to be clear on compare match and set on timer/counter
	// overflow (non-inverting mode).
	TCCR1A = (1 << COM1B1) | (0 << COM1B0) | (1 << WGM11) | (1 << WGM10);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);

	// PWM output should now be happening - at the frequency and pulse width set above
}

ISR(TIMER1_COMPA_vect)
{
	DDRD = (1 << 4);

	dutycycle = duty_percentage;

	if (pulsewidth == 0)
	{
		OCR1B = 0;
	}
	else
	{
		// The compare value is one less than the number of clock cycles in the pulse width
		OCR1B = pulsewidth - 1;
	}

	/* if (button_pressed_during_this_beat)
	I'm pretty sure this should work because btn_pressed_during_this_beat reverts back to 0
	at the starts of every beat, so the audio would only last for the duration of a beat
	AND when a button is pressed. I can't figure out how to implement this though, it seems
	to compile correctly but the LED matrix only shows 'ERR'. */
	if (1)
	{
		// Work out the clock period and pulse width
		clockperiod = freq_to_clock_period(freq);
		pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

		if (pulsewidth == 0)
		{
			OCR1B = 0;
		}
		else
		{
			// The compare value is one less than the number of clock cycles in the pulse width
			OCR1B = pulsewidth - 1;
		}

		OCR1A = clockperiod - 1;
	}
	else
	{
		DDRD = 8;
	}
}