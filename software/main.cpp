/* File: main.cpp
 * Contains base main function and usually all the other stuff that avr does...
 */
/* Copyright (c) 2012-2013 Domen Ipavec (domen.ipavec@z-v.si)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//#include <util/delay.h>

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include <stdint.h>

#include "bitop.h"
#include "lcds.h"

extern "C" {
#include "ds18b20.h"
}

using namespace avr_cpp_lib;

static volatile uint8_t up = 0;
static volatile uint8_t enter = 0;
static volatile uint8_t down = 0;

static InputPin ip_up(&DDRA, &PINA, PA6);
static InputPin ip_enter(&DDRA, &PINA, PA5);
static InputPin ip_down(&DDRA, &PINA, PA4);

ISR(TIMER1_OVF_vect) {
	if (!ip_up.isSet()) {
		up++;
	} else {
		up = 0;
	}
	if (!ip_enter.isSet()) {
		enter++;
	} else {
		enter = 0;
	}
	if (!ip_down.isSet()) {
		down++;
	} else {
		down = 0;
	}
}

static volatile bool temp_trigger = false;
ISR(TIMER2_OVF_vect) {
	temp_trigger = true;
}

static inline void write_temperature(LCDS *lcd, uint8_t y, uint16_t temp) {
	uint8_t dec = 0;
	if (BITSET(temp, 3)) {
		dec += 5;
	}
	if (BITSET(temp, 2)) {
		dec += 3;
	} else if (BITSET(temp, 0)) {
		dec += 1;
	}
	if (BITSET(temp, 1)) {
		dec += 1;
	}
	lcd->gotoXY(7, y);
	lcd->write(temp>>4, 3);
	lcd->write(",");
	lcd->write(dec, 1);
	lcd->write("\xDF""C");
}

static const uint8_t SHORT_PRESS = 2;
static const uint8_t LONG_PRESS = 20;

static inline bool pressed(volatile uint8_t * sw, uint8_t timing = SHORT_PRESS) {
	if (*sw == timing) {
		while(*sw == timing);
		return true;
	} else {
		return false;
	}
}

static uint16_t EEMEM eeprom_alarm_temperature;
static uint8_t EEMEM eeprom_alarm_off;

static inline void update_alarm(LCDS *lcd, uint8_t alarm_off, uint16_t alarm_temperature) {
	if (alarm_off) {
		lcd->gotoXY(7, 1);
		lcd->write("NE     ");
	} else {
		write_temperature(lcd, 1, alarm_temperature);
	}
	eeprom_update_word(&eeprom_alarm_temperature, alarm_temperature);
	eeprom_update_byte(&eeprom_alarm_off, alarm_off);
}

int main() {
	// init

	// init pull ups for buttons
	PUEA = BIT(PA4) | BIT(PA5) | BIT(PA6);

	// init lcd
	OutputPin op_rs(&DDRA, &PORTA, PA2);
	OutputPin op_e(&DDRA, &PORTA, PA0);
	OutputPin op_d4(&DDRB, &PORTB, PB0);
	OutputPin op_d5(&DDRB, &PORTB, PB1);
	OutputPin op_d6(&DDRB, &PORTB, PB2);
	OutputPin op_d7(&DDRA, &PORTA, PA7);
	LCDS lcd(op_rs, op_e, op_d4, op_d5, op_d6, op_d7);

	// init timer for button reads approx 8ms
	TIMSK1 = BIT(TOIE1);
	TCCR1B = BIT(CS10);

	// init timer for temperature reads and beeping approx 0.5s
	// toggle tocc0 or oc2b on comp match
	TOCPMSA0 = BIT(TOCC0S1);
	TOCPMCOE = BIT(TOCC0OE);
	TCCR2A = BIT(COM2B0);
	// interrupt and timing
	TIMSK2 = BIT(TOIE2);
	TCCR2B = BIT(CS21) | BIT(CS20);

	// set precision
	ds18b20wsp(&PORTA, &DDRA, &PINA, BIT(PA3), NULL, 0, 0, DS18B20_RES11);

	// initial lcd writing
	lcd.command(lcd.CLEAR);
	lcd.command(lcd.CURSOR_OFF);

	lcd.gotoXY(0, 0);
	lcd.write("Temp:");
	lcd.gotoXY(0, 1);
	lcd.write("Alarm:");

	// initialize temp variables
	uint16_t temperature = 1<<4;
	bool temp_convert = false;

	// init temp alarm vars
	uint16_t alarm_temperature = eeprom_read_word(&eeprom_alarm_temperature);
	if (alarm_temperature == 0xffff) {
		alarm_temperature = 23<<4;
	}
	uint8_t alarm_off = eeprom_read_byte(&eeprom_alarm_off);
	bool alarm_active = false;
	bool alarm_pause = false;
	update_alarm(&lcd, alarm_off, alarm_temperature);

	// enable interrupts
	sei();

	for (;;) {
		if (temp_trigger) {
			if (temp_convert) {
				int16_t temp;
				ds18b20read(&PORTA, &DDRA, &PINA, BIT(PA3), NULL, &temp );
				temperature = temp;
				write_temperature(&lcd, 0, temperature);
			}
			ds18b20convert(&PORTA, &DDRA, &PINA, BIT(PA3), NULL);
			temp_convert = true;
			temp_trigger = false;
		}

		if (temperature > alarm_temperature && !alarm_off) {
			if (!alarm_active) {
				SETBIT(DDRA, PA1);
				alarm_active = true;
			}
		} else {
			alarm_active = false;
			alarm_pause = false;
			CLEARBIT(DDRA, PA1);
			lcd.gotoXY(15, 1);
			lcd.write(" ");
		}

		if (!alarm_off && pressed(&up)) {
			alarm_temperature += 1<<4;
			update_alarm(&lcd, alarm_off, alarm_temperature);
		}
		if (!alarm_off && pressed(&down)) {
			alarm_temperature -= 1<<4;
			update_alarm(&lcd, alarm_off, alarm_temperature);
		}
		if (pressed(&enter)) {
			if (alarm_active) {
				alarm_pause = !alarm_pause;
				if (alarm_pause) {
					CLEARBIT(DDRA, PA1);
					lcd.gotoXY(15, 1);
					lcd.write("X");
				} else {
					SETBIT(DDRA, PA1);
					lcd.gotoXY(15, 1);
					lcd.write(" ");
				}
			} else {
				alarm_off = !alarm_off;
				update_alarm(&lcd, alarm_off, alarm_temperature);
			}
		}
	}
}



