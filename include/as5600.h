
#include <stdint.h>

#define AS5600_ADRESS 0x36
#define RAW_POSITION_REG 0x0C

#define DIRECTION_DEVICE_ERROR 0xFF

uint8_t read_direction(){
	
	uint8_t buffer;
  buffer = RAW_POSITION_REG;
  uint8_t res = twi_writeTo(AS5600_ADRESS, &buffer, 1, true);
  if(res !=0)
    return DIRECTION_DEVICE_ERROR;
	delay(10);
	res = twi_readFrom(AS5600_ADRESS, &buffer, 1, true);
	//TODO: Check res == 0, otherwise error
  return buffer & 0x0F;
}
