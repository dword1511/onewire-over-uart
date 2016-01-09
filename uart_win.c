#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "uart.h"

HANDLE hSerial;
DCB    dcb;
char   *words, *buffRead, *buffWrite;
DWORD  dwBytesWritten, dwBytesRead;

int uart_init(char *dev_path) {
  hSerial = CreateFile(dev_path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if(hSerial == INVALID_HANDLE_VALUE) {
    return GetLastError();
  }

  return UART_SUCCESS;
}

void uart_finit(void) {
  if(hSerial == INVALID_HANDLE_VALUE) return;

}

void uart_setb(uint32_t baud) {
  if(hSerial == INVALID_HANDLE_VALUE) return;
  static int dcb_buad;

  switch(baud) {
    case 9600:
      dcb_buad = CBR_9600;
      break;
    case 19200:
      dcb_buad = CBR_19200;
      break;
    case 38400:
      dcb_buad = CBR_38400;
      break;
    case 57600:
      dcb_buad = CBR_57600;
      break;
    case 115200:
      dcb_buad = CBR_115200;
      break;
    default:
      /* Above values should be enough. */
      dcb_buad = CBR_9600;
  }

  dcb.DCBlength       = sizeof(dcb);
  dcb.BaudRate        = dcb_buad;
  dcb.ByteSize        = 8;
  dcb.StopBits        = ONESTOPBIT;
  dcb.Parity          = NOPARITY;
  dcb.fBinary         = TRUE;
  dcb.fDtrControl     = DTR_CONTROL_DISABLE;
  dcb.fRtsControl     = RTS_CONTROL_DISABLE;
  dcb.fOutxCtsFlow    = FALSE;
  dcb.fOutxDsrFlow    = FALSE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fAbortOnError   = TRUE;

  SetCommState(hSerial, &dcb);
}

void uart_putc(unsigned char c) {
  DWORD written;
  WriteFile(hSerial, &c, 1, &written, NULL);
}

unsigned char uart_getc(void) {
  DWORD read;
  if(hSerial == INVALID_HANDLE_VALUE) return 0x00;
  static unsigned char c = 0x00;
  while(!ReadFile(hSerial, &c, 1, &read, NULL));
  return c;
}
