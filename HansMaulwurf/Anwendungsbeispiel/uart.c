#include <avr/io.h>
#include <string.h>

void uart_init(void)
{
//      UCSR0A = 0x20;			//UART Config
//      UCSR0B |= (1<<RXEN0);		//UART RX einschalten
        UCSR0B |= (1<<TXEN0);		//UART RX einschalten
        UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);//Asynchron 8N1
        UBRR0L |= 0xCE;			//Baudrate Config
        UBRR0H |= 0x00;			//Baudrate Config
}

/*      UART Senden     */
unsigned char uart_send(char uart_data[])
{
   unsigned char i;
//   for (i=0; i<32; i++)
   for (i=0; i<strlen(uart_data); i++)
    {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = uart_data[i];
    }
    return UDR0;
}


/*
unsigned char uart_send(char uart_data[32])
{
   unsigned char i;
   for (i=0; i<32; i++)
//   for (i=0; i<sizeof(uart_data); i++)
    {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = uart_data[i];
    }
    return UDR0;
}
*/
