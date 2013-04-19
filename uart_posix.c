#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "uart.h"

#define _POSIX_SOURCE 1
#define FALSE 0
#define TRUE  1

volatile int STOP = FALSE;
int fd = -1;

int uart_init(char *dev_path) {
  fd = open(dev_path, O_RDWR | O_NOCTTY | O_NDELAY);
  if(fd < 0) return fd;

  return UART_SUCCESS;
}

void uart_finit(void) {
  if(fd < 0) return;
  close(fd);
}

void uart_setb(uint32_t baud) {
  if(fd < 0) return;
  static int tio_buad;

  switch(baud) {
    case 9600:
      tio_buad = B9600;
      break;
    case 19200:
      tio_buad = B19200;
      break;
    case 38400:
      tio_buad = B38400;
      break;
    case 57600:
      tio_buad = B57600;
      break;
    case 115200:
      tio_buad = B115200;
      break;
    default:
      /* Above values should be enough. */
      tio_buad = B9600;
  }

  struct termios tio = {
    /*
     * BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
     * CRTSCTS : output hardware flow control
     * CS8     : 8n1 (8bit,no parity,1 stopbit)
     * CLOCAL  : local connection, no modem contol
     * CREAD   : enable receiving characters
     */
    .c_cflag = tio_buad | CS8 | CLOCAL | CREAD,
    /*
     * IGNPAR  : ignore bytes with parity errors
     * ICRNL   : map CR to NL (otherwise a CR input on the other computer
     *           will not terminate input)
     *           otherwise make device raw (no other input processing)
     */
    .c_iflag = 0,
    /* Raw output. */
    .c_oflag = 0,
    /*
     * ICANON  : enable canonical input
     *           disable all echo functionality, and don't send signals to calling program
     * NOFLSH  : Disable flushing of input buffers after interrupt or quit characters
     */
    .c_lflag = NOFLSH,
    /*
     * initialize all control characters 
     * default values can be found in /usr/include/termios.h, and are given
     * in the comments, but we don't need them here
     */
    .c_cc = {0},
  };

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &tio);
}

void uart_putc(unsigned char c) {
  if(fd < 0) return;
  write(fd, &c, 1);
}

unsigned char uart_getc(void) {
  if(fd < 0) return 0x00;
  static unsigned char c = 0x00;
  while(!read(fd, &c, 1));
  return c;
}
