#ifndef _I2C_BQ76940
#define _I2C_BQ76940

#define BQI2CADR	0x08
// 1 ist in bq76940_Balancing für das Ausführen der Schleife reserviert
#define ERROR_START	2
#define ERROR_MT_SLA	3
#define ERROR_MR_SLA	4
#define ERROR_MT_DATA	5
#define ERROR_MR_DATA	6

void 	 i2c_init(void);
uint8_t  i2c_start(void);
void	 i2c_stop(void);
uint8_t  i2c_write(uint8_t reg_adr ,uint8_t data_reg);
uint8_t  i2c_read(uint8_t reg_adr, uint8_t twice);
uint8_t  registerRead(uint8_t reg_adr);
uint16_t registerDoubleRead(uint8_t reg_adr);
uint8_t registerArrayRead(uint8_t reg_adr, uint16_t array[], uint8_t count);
uint8_t  registerWrite(uint8_t reg_adr, uint8_t reg_data);

#endif
