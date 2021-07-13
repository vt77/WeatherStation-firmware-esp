#ifndef _ARDUINO_MOCKUP
#define _ARDUINO_MOCKUP

#include <stdint.h>
#include <iostream>

uint8_t (*twi_writeTo)(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop);
uint8_t (*twi_readFrom)(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop);
void delay(long milliseconds){
    std::cout << "[ARDUINOMOCK][DELAY]"  << milliseconds << std::endl;
}

#endif
