#ifndef _I2C_BQ79649
#define _I2C_BQ79649

#define F_CPU 8000000UL
#include <util/delay.h>
#include <util/twi.h>

#include "uart.h"
#include "i2c_bq76490.h"

void i2c_init(void)
{
	TWSR = 00;	//set Prescaler value to 1
	TWBR = 32;	//set SCL to 100kHz
}

uint8_t i2c_start(void)
{
        TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while  (!(TWCR & (1<<TWINT)));

	//check START condition
	if ((TWSR & 0xF8) != TW_START)
	{
		i2c_stop();
//		uart_send("Fehler: TW_MT_START");
		return ERROR_START;
	}
	return 0;
}

void i2c_stop(void)
{
        TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	_delay_us(6);
}

uint8_t i2c_write(uint8_t reg_adr ,uint8_t data_reg)
{
	//load BQ Device Adress
	TWDR = (BQI2CADR << 1);
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));

	//check successful transmision
	if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
	{
		i2c_stop();
//		uart_send("Fehler: TW_MT_SLA_ACK\n");
		return ERROR_MT_SLA;
	}

	//send reg_adr
	TWDR = reg_adr;
	TWCR = (1<<TWINT) | (1<<TWEN);

	//wait till data is send
	while (!(TWCR & (1<<TWINT)));

	//check successful transmision
	if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
	{
		i2c_stop();
//		uart_send("Fehler: TW_MT_DATA_ACK\n");
		return ERROR_MT_DATA;
	}

	//send data_reg
	TWDR = data_reg;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));

	//check successful transmision
	if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
	{
		i2c_stop();
//		uart_send("Fehler: TW_MT_DATA_ACK\n");
		return ERROR_MT_DATA;
	}

	return 0;
}

uint8_t i2c_read(uint8_t reg_adr, uint8_t twice) // twice must be 0 for 1 byte or 1 for 2+++ bytes
{
	//load BQ Device Adress
	TWDR = (BQI2CADR << 1);
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));

	//check successful transmision
	if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
	{
		i2c_stop();
//		uart_send("Fehler: TW_MT_SLA_ACK\n");
		return ERROR_MT_SLA;
	}

	//send data_reg
	TWDR = reg_adr;
	TWCR = (1<<TWINT) | (1<<TWEN);

	//wait till data is send
	while (!(TWCR & (1<<TWINT)));

	//check successful transmision
	if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
	{
		i2c_stop();
//		uart_send("Fehler: TW_MT_DATA_ACK\n");
		return ERROR_MT_DATA;
	}

	i2c_stop();

	i2c_start();

	//load BQ Device Adress
	TWDR = ((BQI2CADR << 1)+1);
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));

	//check successful transmision
	if ((TWSR & 0xF8) != TW_MR_SLA_ACK)
	{
		i2c_stop();
//		uart_send("Fehler: TW_MR_SLA_ACK\n");
		return ERROR_MR_SLA;

	}

//	TWCR = (1<<TWINT)|(1<<TWEN);			//NACK
//	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);}		//ACK
	TWCR = (1<<TWINT)|(1<<TWEN)|(twice<<TWEA);	//if(twice==1) ACK; | if(twice=0) NACK;
	while (!(TWCR & (1<<TWINT)));

	//read data
	return TWDR;
}

uint8_t registerRead(uint8_t reg_adr)
{
	i2c_start();
	i2c_read(reg_adr, 0);
	i2c_stop();
	return TWDR;
}

uint16_t registerDoubleRead(uint8_t reg_adr)
{
	uint8_t regHI = 0;
	uint8_t regLO = 0;

	i2c_start();

	regHI=i2c_read(reg_adr, 1);

	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	regLO = TWDR;

	i2c_stop();

	return ((regHI << 8)|(regLO));
}

uint8_t registerArrayRead(uint8_t reg_adr, uint16_t array[], uint8_t count)
{
	uint8_t i=0;
	uint8_t error = 0;
	uint8_t regHI = 0;
	uint8_t regLO = 0;

	error|=i2c_start();

	regHI=i2c_read(reg_adr, 1);

	for(;i<count; i++)
	{
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);		//ACK
		while (!(TWCR & (1<<TWINT)));
		regLO = TWDR;

		array[i] = ((regHI << 8)|(regLO));

		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);		//ACK
		while (!(TWCR & (1<<TWINT)));
		regHI = TWDR;

	}

	TWCR = (1<<TWINT)|(1<<TWEN);		//NACK
	while (!(TWCR & (1<<TWINT)));

	array[i+1] = ((regHI << 8)|(TWDR));

	i2c_stop();

	return error;
}


uint8_t registerWrite(uint8_t reg_adr, uint8_t reg_data)
{
	uint8_t error = 0;

	error |= i2c_start();
	error |= i2c_write(reg_adr, reg_data);
	i2c_stop();

	return error; //if error==0; everything is ok
}

#endif
