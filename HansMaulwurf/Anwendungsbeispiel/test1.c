#define F_CPU 8000000UL
#include "globales.h"
#include "bq76940.h"
#include "uart.h"
#include "interrupt.h"
#include "i2c_bq76490.h"
#include "bq76940.c"


int main(void)
{

	int16_t i=0;
	uint16_t cellV[15] = {0};
	uint16_t data  = 0;
	uint16_t packV = 0;

	cli();	// Interrupts global deaktiviert

	DDRC  = 0xFF;			// Port C als Ausgangsport
	DDRD  = 0b11110010;		// set PD3(INT1(ALERT)) to input
	PORTD &= ~(1 << PD3);		// set input(PD3) to low (high if fault detected)

uart_init();
	i2c_init();
	if(bq76940_init()) { uart_send("BQ_init fails"); return 1; }
	interrupt_init();

//	sei();	// Interrupts global aktiviert


// Zellenladeausgleich deaktivieren
	registerWrite(CELLBAL1, 0);
	registerWrite(CELLBAL2, 0);
	registerWrite(CELLBAL3, 0);


// Temperaturausgabe
	data = TempRead(0);
	sprintf(hex_s, "Ti: %i \n", data);
	uart_send(hex_s);

	data = TempRead(1);
	sprintf(hex_s, "Te: %u \n", data);
	uart_send(hex_s);

	data = TempRead(2);
	sprintf(hex_s, "Te: %u \n", data);
	uart_send(hex_s);

	data = TempRead(3);
	sprintf(hex_s, "Te: %u \n", data);
	uart_send(hex_s);



// Ausgabe bestimmter Register
	data = registerRead(SYS_STAT);
	sprintf(hex_s, "STAT: %02x\n", data);
	uart_send(hex_s);
	data = registerRead(SYS_CTRL1);
	sprintf(hex_s, "CTRL1: %02x\n", data);
	uart_send(hex_s);
	data = registerRead(SYS_CTRL2);
	sprintf(hex_s, "CTRL2: %02x\n", data);
	uart_send(hex_s);

	return 0;
}
////////////////////////////// MAIN-ENDE /////////////////////////////////////

/*
	registerWrite(SYS_CTRL2, (registerRead(SYS_CTRL2) | (CHG_ON)));		// set
	data = registerRead(SYS_CTRL2);
	sprintf(hex_s, "CTRL2: %x\n", data);
	uart_send(hex_s);

	_delay_ms(10000);
	registerWrite(SYS_CTRL2, (registerRead(SYS_CTRL2) & ~(CHG_ON)));	// clear
	registerWrite(SYS_CTRL2, (registerRead(SYS_CTRL2) & ~(DSG_ON)));	// clear
	data = registerRead(SYS_CTRL2);
	sprintf(hex_s, "CTRL2: %x\n", data);
	uart_send(hex_s);
*/

/*
	registerWrite(SYS_STAT, CC_READY);
	registerWrite(SYS_STAT, DEVICE_XREADY);
	registerWrite(SYS_STAT, OVRD_ALERT);
	registerWrite(SYS_STAT, UV);
	registerWrite(SYS_STAT, OV);
	registerWrite(SYS_STAT, SCD);
	registerWrite(SYS_STAT, OCD);
*/

/*
	// check if INT1 is high; check whats wrong; clear bit
	if(EIFR & (1 << INTF1))
	{
		uart_send("INTF1 in set");
		checkSTAT();
		//clear INTF1 // write 1 into it
		EIFR |= (1 << INTF1);
	}
*/

/*	data = TempRead(0);
	sprintf(hex_s, "T: %i\n ", data);
	uart_send(hex_s);
*/

/*
/////////////// Entladen auf 58.5 VOLT/////////////
while(1) {
packV = PackVoltageRead();
	do
	{
		CellArrayVoltageRead(cellV);
		for(i=0;i<15; i++)
		{
			sprintf(hex_s, "%u; ", cellV[i]);
			uart_send(hex_s);
		}
		packV = PackVoltageRead();
		sprintf(hex_s, "%u\n", packV);
		uart_send(hex_s);

		//Entladen ein
		registerWrite(SYS_CTRL2, DSG_ON);

	}while(packV >= 58500); // 58.5 VOLT

	//Entladen ein
	registerWrite(SYS_CTRL2, 0);
}
////////////////////ENTLADEN//////////////////////
*/


/*
while(1) // Balancing
{
	data = Balancing(cellV);
//	_delay_ms(1000);
	for(i=0;i<15; i++)
	{
		sprintf(hex_s, "%u; ", cellV[i]);
		uart_send(hex_s);
	}
	packV = PackVoltageRead();
	sprintf(hex_s, "%u\n", packV);
	uart_send(hex_s);
}
*/


/*	registerWrite(SYS_STAT, 0x00);

	data = registerRead(SYS_STAT);
	sprintf(hex_s, "REG: %x\n", data);
	uart_send(hex_s);

	sprintf(hex_s, "offset: %i\n", offset);
	uart_send(hex_s);
	sprintf(hex_s, "gain: %i\n", gain);
	uart_send(hex_s);

*/


/*	data = registerRead(CELLBAL1);
	sprintf(hex_s, "data1: %x\n", data);
	uart_send(hex_s);
	data = registerRead(CELLBAL2);
	sprintf(hex_s, "data2: %x\n", data);
	uart_send(hex_s);
	data = registerRead(CELLBAL3);
	sprintf(hex_s, "data3: %x\n", data);
	uart_send(hex_s);

	data = CellVoltageRead(VC15_HI);
	sprintf(hex_s, "cell: %u\n", data);
	uart_send(hex_s);
*/
