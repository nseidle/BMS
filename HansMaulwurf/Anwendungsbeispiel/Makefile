
MCU=atmega168
CC=avr-gcc
OBJCOPY=avr-objcopy

Mikrocontrollername_fuer_avrdude=m168p

PROGRAMMER=avrisp2

#-------------------
# optimize for size:
#CFLAGS=-g -mmcu=$(MCU) -Wall -Wunused-but-set-variable -Os -mcall-prologues
CFLAGS=-g -mmcu=$(MCU) -Wall -Os -mcall-prologues -Wunused-variable

TARGET= test1.c uart.c i2c_bq76490.c


#-------------------
all: test1.hex

#-------------------
help: 
	clear
	@echo
	@echo "Moegliche Befehle:"
	@echo "		make - compiliert Dein Programm und erzeugt die .hex-Datei"
	@echo "		make all - wie make"
	@echo "		make load - compiliert Dein Programm und schiebt es in den AVR"
	@echo "		make clean - loescht die beim Compilieren erzeugten Dateien"
	@echo
	@echo "		make help - zeigt diesen Hilfetext"
	@echo
	@echo "		make rdfuses - gibt Dir Informationen ueber die gesetzten Fusebits und mehr"
	@echo "		make wrfuse4mhz - internen Oszillator auf 4 MHz setzen"
	@echo "		make wrfuse8mhz - internen Oszillator auf 8 MHz setzen"
	@echo "		make wrfusedefaults - Fuses auf Werkseinstellungen zuruecksetzen"
	@echo
	
#-------------------
test1.hex : test1.out 
	$(OBJCOPY) -R .eeprom -O ihex test1.out test1.hex 
test1.out : test1.o 
	$(CC) $(CFLAGS) -o test1.out -Wl,-Map,test1.map test1.o uart.o i2c_bq76490.o
test1.o : test1.c
#	$(CC) $(CFLAGS) -Os -Wunused-but-set-variable -c test1.c
#	$(CC) $(CFLAGS) -Os -c test1.c
	$(CC) $(CFLAGS) -Os -c $(TARGET)
	
#------------------
load: test1.hex
	avrdude -p $(Mikrocontrollername_fuer_avrdude) -c $(PROGRAMMER) -e -U flash:w:"test1.hex"
	
#-------------------
clean:
	rm -f *.o *.map *.out *.hex
	
#-------------------
