/* File: lcds.cpp
 * Synchronous lcd hd44780 interface.
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

#include "lcds.h"
#include "bitop.h"

#include <util/delay.h>
#include <avr/pgmspace.h>

avr_cpp_lib::LCDS::LCDS(OutputPin rs, OutputPin e, OutputPin d4, OutputPin d5, OutputPin d6, OutputPin d7)
	: rs(rs), e(e), d4(d4), d5(d5), d6(d6), d7(d7) {
	_delay_ms(50);

	d7.clear();
	d6.clear();
	d5.set();
	d4.set();
	rs.clear();

	// one (8 bit mode)
	enableToggle();
	_delay_ms(4.5);

	// two (8 bit mode)
	enableToggle();
	_delay_ms(4.5);

	// three (4 bit mode)
	d4.clear();
	enableToggle();
	_delay_us(150);

	// dual line, 4 bit mode
	command(0b00101000);

	// turn display on, cursor off
	command(DISPLAY_ON);

	// clear display
	command(CLEAR);
}

void avr_cpp_lib::LCDS::enableToggle() {
	e.clear();
	_delay_us(1);
	e.set();
	_delay_us(1);
	e.clear();
	_delay_us(100);
}

void avr_cpp_lib::LCDS::command(uint8_t c) {
	rs.clear();
	send(c);
	if (c < 4) { // clear (0b1) and home (0b10)
		_delay_ms(2);
	}
}

void avr_cpp_lib::LCDS::character(uint8_t c) {
	rs.set();
	send(c);
}

void avr_cpp_lib::LCDS::send(uint8_t c) {
	if (BITSET(c, 7)) {
		d7.set();
	} else {
		d7.clear();
	}
	if (BITSET(c, 6)) {
		d6.set();
	} else {
		d6.clear();
	}
	if (BITSET(c, 5)) {
		d5.set();
	} else {
		d5.clear();
	}
	if (BITSET(c, 4)) {
		d4.set();
	} else {
		d4.clear();
	}
	enableToggle();
	if (BITSET(c, 3)) {
		d7.set();
	} else {
		d7.clear();
	}
	if (BITSET(c, 2)) {
		d6.set();
	} else {
		d6.clear();
	}
	if (BITSET(c, 1)) {
		d5.set();
	} else {
		d5.clear();
	}
	if (BITSET(c, 0)) {
		d4.set();
	} else {
		d4.clear();
	}
	enableToggle();
}

void avr_cpp_lib::LCDS::gotoXY(uint8_t x, uint8_t y) {
	uint8_t tmp;
	switch (y) {
	case 0:
		tmp = x;
		break;
	case 1:
		tmp = 0x40 + x;
		break;
	case 2:
		tmp = 0x14 + x;
		break;
	default:
		tmp = 0x54 + x;
	}
	command(1<<7 | tmp);
}

void avr_cpp_lib::LCDS::write(uint32_t i, uint8_t l, char fill, uint8_t m) {
	char buf[l + 1];
	buf[l] = '\0';
	for (; l > 0; l--) {
		uint8_t c = i % m;
		if (c < 10) {
			buf[l-1] = 0x30 + c;
		} else {
			buf[l-1] = 0x41 + c - 10;
		}
		i = i / m;
		if (i <= 0) {
			break;
		}
	}
	for (l--; l > 0; l--) {
		buf[l-1] = fill;
	}
	write(buf);
}

void avr_cpp_lib::LCDS::write(const char * s) {
	while (*s != '\0') {
		character(*s);
		s++;
	}
}

void avr_cpp_lib::LCDS::writeFlash(const char * s) {
	uint8_t c = pgm_read_byte(s);
	while (c != '\0') {
		character(c);
		s++;
		c = pgm_read_byte(s);
	}
}
