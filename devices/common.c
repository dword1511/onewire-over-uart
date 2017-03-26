#include <inttypes.h>

#define CRC8POLY 0x18 // 0x18 = X ^ 8 + X ^ 5 + X ^ 4 + X ^ 0

uint8_t crc8(uint8_t *data_in, uint16_t number_of_bytes_to_read) {
  uint8_t crc = 0x00;
  uint16_t loop_count;
  uint8_t bit_counter;
  uint8_t data;
  uint8_t feedback_bit;

  for(loop_count = 0; loop_count != number_of_bytes_to_read; loop_count++) {
    data = data_in[loop_count];
    bit_counter = 8;
    do {
      feedback_bit = (crc ^ data) & 0x01;
      if ( feedback_bit == 0x01 ) crc = crc ^ CRC8POLY;
      crc = (crc >> 1) & 0x7F;
      if ( feedback_bit == 0x01 ) crc = crc | 0x80;
      data = data >> 1;
      bit_counter--;
    } while(bit_counter > 0);
  }

  return crc;
}

/*
For crc8():

This code is from Colin O'Flynn - Copyright (c) 2002
only minor changes by M.Thomas 9/2004

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

void delay_ms(uint32_t ms) {
#ifdef _WIN32
  Sleep(ms);
#else
  struct timespec tv = {
    .tv_sec  = ms / 1000,
    .tv_nsec = (ms % 1000) * 1000000,
  };
  nanosleep(&tv, NULL);
#endif
}
