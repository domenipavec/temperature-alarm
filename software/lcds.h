/* File: lcds.h
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
#ifndef LCDS_H
#define LCDS_H

#include <stdint.h>

#include "io.h"

namespace avr_cpp_lib {

	class LCDS {
	public:
		LCDS(OutputPin rs, OutputPin e, OutputPin d4, OutputPin d5, OutputPin d6, OutputPin d7);
		void write(const char * s);
		void writeFlash(const char * s);
		void write(uint32_t i, uint8_t l, char fill = ' ', uint8_t m = 10);
		void gotoXY(uint8_t x, uint8_t y);
		void command(uint8_t c);
		void character(uint8_t c);

		static const uint8_t CLEAR = (1<<0);
		static const uint8_t HOME = (1<<1);
		static const uint8_t CURSOR_ON = 0b00001110;
		static const uint8_t CURSOR_ON_BLINK = 0b00001111;
		static const uint8_t CURSOR_BLINK = 0b00001101;
		static const uint8_t CURSOR_OFF = 0b00001100;
		static const uint8_t CURSOR_LEFT = 0b00010000;
		static const uint8_t CURSOR_RIGHT = 0b00010100;
		static const uint8_t DISPLAY_ON = 0b00001100;
		static const uint8_t DISPLAY_OFF = 0b00001000;
		static const uint8_t DISPLAY_RIGHT = 0b00011100;
		static const uint8_t DISPLAY_LEFT = 0b00011000;
	private:
		void enableToggle();
		void send(uint8_t c);

		OutputPin rs;
		OutputPin e;
		OutputPin d4;
		OutputPin d5;
		OutputPin d6;
		OutputPin d7;
	};

}

#endif
