#ifndef _UART_ATMEGA169
#define _UART_ATMEGA169

void uart_init(void);
unsigned char uart_send(char uart_data[32]);

#endif
