#ifndef _INTERRUPT
#define _INTERRUPT

#include "globales.h"
#include "i2c_bq76490.h"

void interrupt_init(void)
{
	EICRA = (1 << ISC11)|(1 << ISC10);	// interrupt at rising edge
	EIMSK = (1 << INT1);			// enable Interrupt #1
}

void checkSTAT(void)
{
	uint8_t stat = registerRead(SYS_STAT);

// Debug
//sprintf(hex_s, "INTER_STAT: %x\n", stat);
//uart_send(hex_s);

	if(stat & (CC_READY))		// ADC for current is ready
	{
//		uart_send("CC READY\n");

		current = CurrentRead();
	}

	if(stat & (OVRD_ALERT))	// external fault detected
	{
//		uart_send("Externer Fehler\n");
	}

	if(stat & (DEVICE_XREADY))	// internal fault
	{
//		uart_send("Interner Fehler\n");

		// check internal error
		if(stat & (UV))		// undervoltage
		{
//			uart_send("Entladeschlussspg. erreicht!\n");
		}
		if(stat & (OV))		// overvoltage
		{
//			uart_send("Ladeschlussspg. erreicht!\n");
		}
		if(stat & (SCD))	// short circuit
		{
//			uart_send("Kurzschluss!\n");
		}
		if(stat & (OCD))	// over current
		{
//			uart_send("Ueberstrom!\n");
		}
	}

// Debug
//sprintf(hex_s, "Write: %x \n", stat);
//uart_send(hex_s);
_delay_ms(5000);

	registerWrite(SYS_STAT, stat); // clear SYS_STAT to pulldown ALERT

}

// Interrupt Routine if Alert is high
ISR(INT1_vect)
{
	checkSTAT();
}

#endif
