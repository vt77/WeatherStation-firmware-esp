#include <stdint.h>

#define SENSOR_BME280

//Sensor settings
//According to datasheet for weather monitoring use forced mode 1 sample/minute
//Oversampling pressure ×1, temperature ×1, humidity ×1
//pressure noise is of no concern

#define BME280_MODE 0x01  //Forced Normal
#define BME280_SAMPLING_TEMP 0x01  //16bit sampling
#define BME280_SAMPLING_PRES  0x01  //16bit sampling
#define BME280_SAMPLING_HUM  0x01  //16bit sampling
#define BME280_STANDBYTIME 0x05  //1 Sec
#define BME280_FILTER 0x00  //0..0x04 =>  0 - OFF 0x04 - x16 


#define SENSOR_ADDRESS 0x76
#define BME280_REG_ID 0xD0
#define BME280_ID 0x60

#define BME280_REG_SOFTRESET 0xE0 //BME280 SOFT RESET REGISTER
#define BME280_SOFTRESET_VALUE 0xB6 //BME280 SOFT RESET VALUE

#define VALUE_ERROR 0xFF


typedef struct __attribute__((packed, aligned(1)))
{
  //Temperature calibration
  uint16_t  dig_T1;  // 0x88/0x89
  int16_t   dig_T2;
  int16_t   dig_T3;

  //Pressure calibration
  uint16_t  dig_P1;
  int16_t   dig_P2;
  int16_t   dig_P3;
  int16_t   dig_P4;
  int16_t   dig_P5;
  int16_t   dig_P6;
  int16_t   dig_P7;
  int16_t   dig_P8;
  int16_t   dig_P9;  // 0x9E/0x9F
  
  //Humidity calibration
  uint8_t   reg_a0;    //0xA0 - not used 
  uint8_t   dig_H1;    //0xA1 - 0x88 - 0xA1 total 26
  int16_t   dig_H2;    //0xE1/E2
  uint8_t   dig_H3;    //0xE3
  int16_t   dig_H4;    //0xE4/0xE5[3:0]
  int16_t   dig_H5;    //E5[7:4]/e6
  int8_t    dig_H6;    //0xE7
} BME280_CalibData;

typedef struct
{
  uint32_t pres;
  int32_t  temp;
  uint32_t hum;
}BME280_SensorsData;


#define BME280_STATUS_IM_UPDATE 0X01 //NVM data copying
#define BME280_REGISTER_CALIB_START 0x88
#define BME280_REGISTER_STATUS 0xF3
#define BME280_REGISTER_CONTROL 0xF4
#define BME280_REGISTER_CONTROL_HUM 0xF2
#define BME280_REGISTER_CONFIG 0xF5 // Configuration register
#define BME280_REGISTER_CHIPID 0xD0 // 0x60 for BME 0x5[678] for BMP 
#define BME280_REGISTER_PRESSUREDATA 0xF7
#define BME280_REGISTER_TEMPDATA 0xFA
#define BME280_REGISTER_HUMIDDATA 0xFD


BME280_CalibData _bme280_calib;
BME280_SensorsData _sensorData;
int32_t t_fine;

uint32_t read24(uint8_t addr)
{
	  uint32_t buffer;
    twi_writeTo(SENSOR_ADDRESS, &addr, 1, true);
    twi_readFrom(SENSOR_ADDRESS, (uint8_t*)&buffer, 3, true);
    return ((buffer>>16)&0x000000FF)|(buffer&0x0000FF00)|((buffer<<16)&0x00FF0000);
}

void write8(uint8_t reg, uint8_t value)
{ 
    uint8_t buffer[2] = {reg,value};
    twi_writeTo(SENSOR_ADDRESS, buffer, 2, true);
}


void read_calibration_data()
{

  uint8_t* calib_ptr = (uint8_t*)&_bme280_calib;
  //Read calibration data calib00…calib25 0x88 - 0xA1
  uint8_t addr = 0x88;
  twi_writeTo(SENSOR_ADDRESS, &addr, 1, true);
  twi_readFrom(SENSOR_ADDRESS, calib_ptr , 26, true);
  //calib26…calib41  0xE1 - 0xF0
  addr = 0xE1;
  twi_writeTo(SENSOR_ADDRESS, &addr, 1, true);
  twi_readFrom(SENSOR_ADDRESS, (uint8_t*)(calib_ptr + 26) , 7, true);

  //Serial.print("[BME280][CALIB][BEFOREFIX] 0x");
  //Serial.print(_bme280_calib.dig_H4,HEX);
  //Serial.print(" 0x");
  //Serial.print(_bme280_calib.dig_H5,HEX);

  //Fix  
  // dig_H4 0xE4/0xE5[3:0]
  // dig_H5 0xE5[7:4]/0xE6
  // dig_H6;    //0xE7

  _bme280_calib.dig_H6 = (int8_t)_bme280_calib.dig_H6 & 0xFF; 
  _bme280_calib.dig_H5 = _bme280_calib.dig_H5 >> 8 | (_bme280_calib.dig_H4 & 0xF0 << 4);
  _bme280_calib.dig_H4 = _bme280_calib.dig_H4 >> 4;

  /*
  Serial.print("[BME280][CALIB][TEMP] 0x");
  Serial.print(_bme280_calib.dig_T1,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_T2,HEX);
  Serial.print(" 0x");
  Serial.println(_bme280_calib.dig_T3,HEX);

  Serial.print("[BME280][CALIB][PRE] 0x");
  Serial.print(_bme280_calib.dig_P1,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_P2,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_P3,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_P4,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_P5,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_P6,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_P7,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_P8,HEX);
  Serial.print(" 0x");
  Serial.println(_bme280_calib.dig_P9,HEX);

  Serial.print("[BME280][CALIB][HUM] 0x");
  Serial.print(_bme280_calib.dig_H1,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_H2,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_H3,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_H4,HEX);
  Serial.print(" 0x");
  Serial.print(_bme280_calib.dig_H5,HEX);
  Serial.print(" 0x");
  Serial.println(_bme280_calib.dig_H6,HEX);
  */


}

uint8_t sensor_start()
{
  uint8_t flags = 0, addr = 0;
  for(int i=0;i<128;i++)
  {
      flags = BME280_REGISTER_CHIPID;
      addr = (uint8_t)i;
      Serial.print("Scan address 0x");
      Serial.print(addr,HEX);
      int res = twi_writeTo(addr, &flags, 1, true);  
      Serial.print("  Status : ");
      Serial.println(res);
  }

  flags = BME280_REGISTER_CHIPID;
  int res = twi_writeTo(SENSOR_ADDRESS, &addr, 1, true);
  twi_readFrom(SENSOR_ADDRESS, (uint8_t*)&flags, 1, true);

  Serial.print("Write status : ");
  Serial.println(res);
  Serial.print("Sensor id : 0x");
  Serial.println(flags,HEX);

  read_calibration_data();

  //Set config
  uint8_t _config =  ( BME280_STANDBYTIME << 5 ) | (BME280_FILTER << 2);
  write8(BME280_REGISTER_CONFIG, _config);
  //Set sampling
  write8(BME280_REGISTER_CONTROL_HUM, BME280_SAMPLING_HUM); 
  uint8_t _meas =  ( BME280_SAMPLING_TEMP << 5 ) | ( BME280_SAMPLING_PRES << 2 ) | BME280_MODE;
  write8(BME280_REGISTER_CONTROL, _meas);

  return flags;
}

void sensor_bust_read()
{
	  uint8_t buffer[8];

    //Set sampling
    write8(BME280_REGISTER_CONTROL_HUM, BME280_SAMPLING_HUM);
    uint8_t _meas =  ( BME280_SAMPLING_TEMP << 5 ) | ( BME280_SAMPLING_PRES << 2 ) | BME280_MODE;
    write8(BME280_REGISTER_CONTROL, _meas);
    
    //TODO: Read status register
    delay(50);

    uint8_t addr = 0xF7;  //Start of data address - 3 bytes pressure, 3 bytes temp , 2 bytes hum
    twi_writeTo(SENSOR_ADDRESS, &addr, 1, true);
    twi_readFrom(SENSOR_ADDRESS, (uint8_t*)&buffer, 8, true);

    _sensorData.pres = (buffer[0] << 12) | (buffer[1] << 4) | ( buffer[2] >> 4 );
    _sensorData.temp = (buffer[3] << 12) | (buffer[4] << 4) | ( buffer[5] >> 4 );
    _sensorData.hum = (buffer[6] << 8) | buffer[7];

     Serial.print("[BME280]T:0x");
     Serial.print(_sensorData.temp,HEX);
     Serial.print(" P:0x");
     Serial.print(_sensorData.pres,HEX);
     Serial.print(" H:0x");
     Serial.println(_sensorData.hum,HEX);
}


float  get_humidity()
{  
    //Humidity is always 16bit resolution
    //uint16_t adc_H = read16(BME280_REGISTER_HUMIDDATA);
    int16_t adc_H = _sensorData.hum;

    uint32_t var1 = (t_fine - ((int32_t)76800));
    var1 = (((((adc_H << 14) - (((int32_t)_bme280_calib.dig_H4) << 20) - (((int32_t)_bme280_calib.dig_H5) * var1)) +
    ((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)_bme280_calib.dig_H6)) >> 10) * (((var1 * ((int32_t)_bme280_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
    ((int32_t)_bme280_calib.dig_H2) + 8192) >> 14));
    var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)_bme280_calib.dig_H1)) >> 4));

    var1 = (var1 < 0 ? 0 : var1);
    var1 = (var1 > 419430400 ? 419430400 : var1);

    float hum_float = (float)(var1>>12) / 1024.0;
    return (float)hum_float;    
}

float get_pressure()
{
    //BME280_U32_t adc_P = read24(BME280_REGISTER_PRESSUREDATA);
    int32_t adc_P = _sensorData.pres;
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)_bme280_calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)_bme280_calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)_bme280_calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)_bme280_calib.dig_P3) >> 8) +
          ((var1 * (int64_t)_bme280_calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)_bme280_calib.dig_P1) >> 33;

    if (var1 == 0) {
      return 0; // avoid exception caused by division by zero
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)_bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)_bme280_calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)_bme280_calib.dig_P7) << 4);

    //p is a pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits)
    return (float)p / 256;
}

float get_temperature(){

  sensor_bust_read();

  int32_t var1, var2;
  //int32_t adc_T = read24(BME280_REGISTER_TEMPDATA);
  uint32_t adc_T = _sensorData.temp;

  var1  = ((((adc_T>>3) - ((int32_t)_bme280_calib.dig_T1 <<1))) * 
        ((int32_t)_bme280_calib.dig_T2)) >> 11;
  var2  = (((((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1)) *
	     ((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1))) >> 12) *
	   ((int32_t)_bme280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;
  int32_t T  = (t_fine * 5 + 128) >> 8;
  return T/100.0;
}
