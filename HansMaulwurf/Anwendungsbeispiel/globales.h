#ifndef _GLOBALES
#define _GLOBALES

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint16_t gain   = 0;
int8_t  offset  = 0;
int16_t current = 0;

//Debug
char hex_s[32] = {0};

#endif
