#ifndef _BQ76940
#define _BQ76940

// Register addresses
#define SYS_STAT    0x00
#define CELLBAL1    0x01
#define CELLBAL2    0x02
#define CELLBAL3    0x03
#define SYS_CTRL1   0x04
#define SYS_CTRL2   0x05
#define PROTECT1    0x06
#define PROTECT2    0x07
#define PROTECT3    0x08
#define OV_TRIP     0x09
#define UV_TRIP     0x0A
#define CC_CFG      0x0B
//Cells
#define VC1_HI  0x0C
#define VC1_LO  0x0D
#define VC2_HI  0x0E
#define VC2_LO  0x0F
#define VC3_HI  0x10
#define VC3_LO  0x11
#define VC4_HI  0x12
#define VC4_LO  0x13
#define VC5_HI  0x14
#define VC5_LO  0x15
#define VC6_HI  0x16
#define VC6_LO  0x17
#define VC7_HI  0x18
#define VC7_LO  0x19
#define VC8_HI  0x1a
#define VC8_LO  0x1b
#define VC9_HI  0x1c
#define VC9_LO  0x1d
#define VC10_HI 0x1e
#define VC10_LO 0x1f
#define VC11_HI 0x20
#define VC11_LO 0x21
#define VC12_HI 0x22
#define VC12_LO 0x23
#define VC13_HI 0x24
#define VC13_LO 0x25
#define VC14_HI 0x26
#define VC14_LO 0x27
#define VC15_HI 0x28
#define VC15_LO 0x29
//Other VC registers are done with an offset in software
#define BAT_HI  0x2A
#define BAT_LO  0x2B
#define TS1_HI  0x2C
#define TS1_LO  0x2D
#define TS2_HI  0x2E
#define TS2_LO  0x2F
#define TS3_HI  0x30
#define TS3_LO  0x31
//Ofther TS registers are done with an offset in software
#define CC_HI	  0x32
#define CC_LO	  0x33
#define ADCGAIN1  0x50
#define ADCOFFSET 0x51
#define ADCGAIN2  0x59
//SYS_STAT bit masks
#define CC_READY 1<<7
#define DEVICE_XREADY 1<<5
#define OVRD_ALERT 1<<4
#define UV 1<<3
#define OV 1<<2
#define SCD 1<<1
#define OCD 1<<0
//SYS_CTRL1 bit masks
#define LOAD_PRESENT 1<<7
#define ADC_EN 1<<4
#define TEMP_SEL 1<<3
#define SHUT_A 1<<1
#define SHUT_B 1<<0
//SYS_CTRL2 bit masks
#define DELAY_DIS 1<<7
#define CC_EN 1<<6
#define CC_ONESHOT 1<<5
#define DSG_ON 1<<1
#define CHG_ON 1<<0

//System_Makros
#define NUMBER_OF_CELLS	15	// in Stueck
#define OV_THERSHOLD	4200	// in mV
#define UV_THERSHOLD	2700	// in mV
#define R_SHUNT		3	// in hµV/A (Hekto Micro Volt pro Ampere)
#define ZLSS		4000	// ZellLadeSchlussSpannung in mV
#define ZUELS		30	// ZellUEberLadungsSpannung in mV zur Kompensation der Selbstentladung
#define MAX_TEMP	65	// maximal zulaessige Temperatur der Speicherzellen in °C

//ERROR_Makros
#define ERROR_ADC	255
#define ERROR_XREADY	254
#define ERROR_GAIN	253

uint8_t bq76940_init(void);
uint8_t enterShipMode(void);
int8_t offsetRead(void);
uint16_t gainRead(void);
uint16_t PackVoltageRead(void);
uint16_t CellVoltageRead(uint8_t cellregister);
uint8_t CellArrayVoltageRead(uint16_t cellV[]);
uint8_t tripCalculator(uint32_t thershold);
uint8_t OVtripWrite(void);
uint8_t UVtripWrite(void);
int16_t TempRead(uint8_t number);
int16_t CurrentRead(void);
uint8_t BalancRegCheck(uint8_t bal_reg);
uint16_t BalancRegSplit(uint16_t bal_reg);
uint16_t Balancing(uint16_t cellV[15]);
int16_t RtoT(uint32_t resistance);

#endif
