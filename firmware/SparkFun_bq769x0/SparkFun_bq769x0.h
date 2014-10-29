// Register addresses
#define bq796x0_SYS_STAT    0x00
#define bq796x0_CELLBAL1    0x01
#define bq796x0_CELLBAL2    0x02
#define bq796x0_CELLBAL3    0x03
#define bq796x0_SYS_CTRL1   0x04
#define bq796x0_SYS_CTRL2   0x05
#define bq796x0_PROTECT1    0x06
#define bq796x0_PROTECT2    0x07
#define bq796x0_PROTECT3    0x8
#define bq796x0_OV_TRIP     0x09
#define bq796x0_UV_TRIP     0x0A
#define bq796x0_CC_CFG      0x0B

//Read-only
#define bq796x0_VC1_HI  0x0C
#define bq796x0_VC1_LO  0x0D
//Other VC registers are done with an offset in software
#define bq796x0_BAT_HI  0x2A
#define bq796x0_BAT_LO  0x2B
#define bq796x0_TS1_HI  0x2C
#define bq796x0_TS1_LO  0x2D
//Ofther TS registers are done with an offset in software
#define bq796x0_CC_HI  0x32
#define bq796x0_CC_LO  0x33
#define bq796x0_ADCGAIN1  0x50
#define bq796x0_ADCOFFSET  0x51
#define bq796x0_ADCGAIN2  0x59

//SYS_STAT bit masks
#define bq796x0_CC_READY  1<<7
#define bq796x0_DEVICE_XREADY 1<<5
#define bq796x0_OVRD_ALERT 1<<4
#define bq796x0_UV 1<<3
#define bq796x0_OV 1<<2
#define bq796x0_SCD 1<<1
#define bq796x0_OCD 1<<0

//SYS_CTRL1 bit masks
#define bq796x0_LOAD_PRESENT 1<<7
#define bq796x0_ADC_EN 1<<4
#define bq796x0_TEMP_SEL 1<<3
#define bq796x0_SHUT_A 1<<1
#define bq796x0_SHUT_B 1<<0

//SYS_CTRL2 bit masks
#define bq796x0_DELAY_DIS 1<<7
#define bq796x0_CC_EN 1<<6
#define bq796x0_CC_ONESHOT 1<<5
#define bq796x0_DSG_ON 1<<1
#define bq796x0_CHG_ON 1<<0

