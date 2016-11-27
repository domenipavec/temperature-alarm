/* File: io.cpp
 * Basic input output pin interfaces.
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

#include "io.h"
#include "bitop.h"

using namespace avr_cpp_lib;

OutputPin::OutputPin(uint8_t volatile * const ddr, uint8_t volatile * const port, uint8_t const value)
	:port(port), value(value) {
	SETBIT(*ddr, value);	
}

void OutputPin::set() {
	SETBIT(*port, value);
}

void OutputPin::clear() {
	CLEARBIT(*port, value);
}

void OutputPin::toggle() {
	TOGGLEBIT(*port, value);
}

InputPin::InputPin(uint8_t volatile * const ddr, uint8_t const volatile * const pin, uint8_t const value)
	:pin(pin), value(value) {
	CLEARBIT(*ddr, value);
}

bool InputPin::isSet() {
	return BITSET(*pin, value);
}
