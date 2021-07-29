#include <stdint.h>

#define I2C_SHT21_ADDRESS 0x40
#define COMMAND_TEMPERATURE 0XF3
#define COMMAND_HUMIDITY  0xF5
#define COMMAND_SETTINGS  0xE6
#define COMMAND_SOFT_RESET 0xFE
#define SHT21_CRC8_POLYNOMINAL 0x13100;  // P(x)=x^8+x^5+x^4+1


#ifndef SHT21_ERROR
#define SHT21_ERROR(...)
#endif

#ifndef SHT21_DEBUG
#define SHT21_DEBUG(...)
#endif

typedef enum {
	RES_12_14BIT    = 0x00, // RH=12bit, T=14bit
	RES_8_12BIT     = 0x01, // RH= 8bit, T=12bit
	RES_10_13BIT    = 0x80, // RH=10bit, T=13bit
	RES_11_11BIT    = 0x81, // RH=11bit, T=11bit
        OTP_RELOAD      = 0x02,  //OTP reload bit
	RES_MASK        = 0x81   // Mask for res. bits (7,0) in user reg.
}SHT21UserRegister;


#define VALUE_ERROR 0xFF

uint8_t calc_crc(uint16_t data)
{
  for (uint8_t bit = 0; bit < 16; bit++)
  {
    if(data & 0x8000)
    {
            data = (data << 1) ^ SHT21_CRC8_POLYNOMINAL;
    }else{
            data <<= 1;
    }
  }
  return data >>= 8;
}

uint8_t sht21_sensor_start()
{
        uint8_t buffer[3];
        buffer[0] = COMMAND_SOFT_RESET;
        if(twi_writeTo(I2C_SHT21_ADDRESS, buffer, 1, true) != 0 )
        {
           SHT21_ERROR("[INIT]Device not ready");       
           return 0;
        }
        delay(20);

        buffer[0] = COMMAND_SETTINGS;
        buffer[1] = SHT21UserRegister::RES_11_11BIT | SHT21UserRegister::OTP_RELOAD;
        SHT21_DEBUG("[SETTINGS] 0x%X",buffer[1]);
        twi_writeTo(I2C_SHT21_ADDRESS, buffer, 2, true);

        return (SENSOR_TEMPERATURE|SENSOR_HUMIDITY);
}

float sht21_get_temperature(){
        uint8_t buffer[3];
        buffer[0] = COMMAND_TEMPERATURE;
        twi_writeTo(I2C_SHT21_ADDRESS, buffer, 1, true);
        delay(100);
        twi_readFrom(I2C_SHT21_ADDRESS, buffer, 3, true);
        SHT21_DEBUG("[TEMP] Readed 0x%X 0x%X 0x%X",buffer[0],buffer[1],buffer[2]);
        uint16_t t =  (buffer[0] << 8) + buffer[1];
        uint8_t crc = calc_crc(t);
        if(crc != buffer[2]){
                SHT21_ERROR("[TEMP] CRC Error 0x%X Calculated 0x%X",buffer[2],crc);
                return VALUE_ERROR;
        }
        t &= ~0x0003;
        float temperature = (-46.85 + 175.72/65536 * (float)t); 
        SHT21_DEBUG("[TEMP] %.2f",temperature);
        return temperature;
}

float  sht21_get_humidity(){
        uint8_t buffer[3];
        buffer[0] = COMMAND_HUMIDITY;
        twi_writeTo(I2C_SHT21_ADDRESS, buffer, 1, true);
        buffer[0] = 0x00;
	delay(150);
	for(int i=0;i<5;i++)
	{
        	int res = twi_readFrom(I2C_SHT21_ADDRESS, buffer, 3, true);        
		if(res == 0)
			break;
		delay(50);
	}

        SHT21_DEBUG("[HUM] Readed 0x%X 0x%X 0x%X",buffer[0],buffer[1],buffer[2]);
	uint16_t rh =  (buffer[0] << 8) + buffer[1];
        uint8_t crc = calc_crc(rh);
        if(crc != buffer[2]){
                SHT21_ERROR("[HUM] CRC Error 0x%X Calculated 0x%X",buffer[2],crc);        
                return VALUE_ERROR;
        }
        rh &= ~0x0003;
        float hum = ( 0.001907 * (float)rh - 6); 
        SHT21_DEBUG("[HUM] %.2f",hum);
        return hum;
}

uint16_t sht21_get_pressure()
{
        SHT21_ERROR("[PRESS] Not supported");
        return 0;
}


#ifndef UNIT_TEST
#define SENSOR_START        sht21_sensor_start
#define SENSOR_GET_PRESSURE sht21_get_pressure
#define SENSOR_GET_HUMIDITY sht21_get_humidity
#define SENSOR_GET_TEMPERATURE sht21_get_temperature
#endif

