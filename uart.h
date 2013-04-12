#ifndef __UART_H__
#define __UART_H__

#define UART_SUCCESS 0

/* Settings */
/* NOTE: baud rate lower than 9600 / 15200 might not work */
#ifndef BAUD_LOW
  #define BAUD_LOW  9600
#endif
#ifndef BAUD_HIGH
  #define BAUD_HIGH 115200
#endif

/* UART implemention for different platforms */
int           uart_init(char *dev_path);
void          uart_finit(void);
void          uart_setb(uint32_t baud);
void          uart_putc(unsigned char c);
unsigned char uart_getc(void);

#endif /* __UART_H__ */
