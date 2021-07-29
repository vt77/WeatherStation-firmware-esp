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

#define BME280_SENSOR_ADDRESS 0x76
#define BME280_REG_ID 0xD0
#define BME280_ID 0x60

#define BME280_REG_SOFTRESET 0xE0 //BME280 SOFT RESET REGISTER
#define BME280_SOFTRESET_VALUE 0xB6 //BME280 SOFT RESET VALUE

#define VALUE_ERROR 0xFF

#ifndef BME280_ERROR
#define BME280_ERROR(...)
#endif

#ifndef BME280_DEBUG
#define BME280_DEBUG(...)
#endif


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
  uint32_t temp;
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

void write_byte(uint8_t reg, uint8_t value)
{ 
    uint8_t buffer[2] = {reg,value};
    twi_writeTo(BME280_SENSOR_ADDRESS, buffer, 2, true);
}


uint16_t read_word(uint8_t addr)
{
  uint16_t data = 0;
  twi_writeTo(BME280_SENSOR_ADDRESS, &addr, 1, true);
  twi_readFrom(BME280_SENSOR_ADDRESS, (uint8_t *)&data, 2, true);
  return data;
}

void read_calibration_data()
{

  /*
  _bme280_calib.dig_T1 = read_word(0x88);
  _bme280_calib.dig_T2 = read_word(0x8A);
  _bme280_calib.dig_T3 = read_word(0x8C);
  _bme280_calib.dig_P1 = read_word(0x8E);
  _bme280_calib.dig_P2 = read_word(0x90);
  _bme280_calib.dig_P3 = read_word(0x92);
  _bme280_calib.dig_P4 = read_word(0x94);
  _bme280_calib.dig_P5 = read_word(0x96);
  _bme280_calib.dig_P6 = read_word(0x98);
  _bme280_calib.dig_P7 = read_word(0x9A);
  _bme280_calib.dig_P8 = read_word(0x9C);
  _bme280_calib.dig_P9 = read_word(0x9E);
*/
  
  uint8_t* calib_ptr = (uint8_t*)&_bme280_calib;
  //Read calibration data calib00…calib25 0x88 - 0xA1
  uint8_t addr = 0x88;
  twi_writeTo(BME280_SENSOR_ADDRESS, &addr, 1, true);
  twi_readFrom(BME280_SENSOR_ADDRESS, calib_ptr , 26, true);

  //calib26…calib41  0xE1 - 0xF0
  //addr = 0xE1;
  //twi_writeTo(BME280_SENSOR_ADDRESS, &addr, 1, true);
  //twi_readFrom(BME280_SENSOR_ADDRESS, (uint8_t*)(calib_ptr + 26) , 7, true);

  //Fix  
  // dig_H4 0xE4/0xE5[3:0]
  // dig_H5 0xE5[7:4]/0xE6
  // dig_H6;    //0xE7

  _bme280_calib.dig_H6 = (int8_t)_bme280_calib.dig_H6 & 0xFF; 
  _bme280_calib.dig_H5 = _bme280_calib.dig_H5 >> 8 | (_bme280_calib.dig_H4 & 0xF0 << 4);
  _bme280_calib.dig_H4 = _bme280_calib.dig_H4 >> 4;


  BME280_DEBUG("[CALIB][T1] %d",_bme280_calib.dig_T1);
  BME280_DEBUG("[CALIB][T2] %d",_bme280_calib.dig_T2);
  BME280_DEBUG("[CALIB][T3] %d",_bme280_calib.dig_T3);
  BME280_DEBUG("[CALIB][P1] %d",_bme280_calib.dig_P1);
  BME280_DEBUG("[CALIB][P2] %d",_bme280_calib.dig_P2);
  BME280_DEBUG("[CALIB][P3] %d",_bme280_calib.dig_P3);
  BME280_DEBUG("[CALIB][P4] %d",_bme280_calib.dig_P4);
  BME280_DEBUG("[CALIB][P5] %d",_bme280_calib.dig_P5);
  BME280_DEBUG("[CALIB][P6] %d",_bme280_calib.dig_P6);
  BME280_DEBUG("[CALIB][P7] %d",_bme280_calib.dig_P7);
  BME280_DEBUG("[CALIB][P8] %d",_bme280_calib.dig_P8);
  BME280_DEBUG("[CALIB][P9] %d",_bme280_calib.dig_P9);

}

uint8_t bme280_sensor_start()
{
  uint8_t flags = 0, addr = 0, chipid = 0;

  BME280_DEBUG("Start");

  addr = BME280_REGISTER_CHIPID;  
  int res = twi_writeTo(BME280_SENSOR_ADDRESS, &addr, 1, true);
  if(res != 0)
  {
      BME280_ERROR("Error read CHIPID %d", res );
      return 0;
  }

  twi_readFrom(BME280_SENSOR_ADDRESS, (uint8_t*)&chipid, 1, true);
  BME280_DEBUG("CHIPID 0x%X",chipid);
  if(0x60 == chipid)
  {
      flags = SENSOR_TEMPERATURE | SENSOR_HUMIDITY | SENSOR_PRESSURE;
  }else{
     //TODO: check 0x5[678]
     flags = SENSOR_TEMPERATURE | SENSOR_PRESSURE;
  }

  read_calibration_data();

  //Set config
  uint8_t _config =  ( BME280_STANDBYTIME << 5 ) | (BME280_FILTER << 2);
  write_byte(BME280_REGISTER_CONFIG, _config);
  //Set sampling
  write_byte(BME280_REGISTER_CONTROL_HUM, BME280_SAMPLING_HUM); 
  uint8_t _meas =  ( BME280_SAMPLING_TEMP << 5 ) | ( BME280_SAMPLING_PRES << 2 ) | BME280_MODE;
  write_byte(BME280_REGISTER_CONTROL, _meas);

  return flags;
}

void sensor_bust_read()
{
	  uint8_t buffer[8];

    //Set sampling
    write_byte(BME280_REGISTER_CONTROL_HUM, BME280_SAMPLING_HUM);
    uint8_t _meas =  ( BME280_SAMPLING_TEMP << 5 ) | ( BME280_SAMPLING_PRES << 2 ) | BME280_MODE;
    write_byte(BME280_REGISTER_CONTROL, _meas);
    
    //TODO: Read status register
    delay(50);

    uint8_t addr = 0xF7;  //Start of data address - 3 bytes pressure, 3 bytes temp , 2 bytes hum
    twi_writeTo(BME280_SENSOR_ADDRESS, &addr, 1, true);
    twi_readFrom(BME280_SENSOR_ADDRESS, (uint8_t*)&buffer, 8, true);

    _sensorData.pres = (buffer[0] << 12) | (buffer[1] << 4) | ( buffer[2] >> 4 );
    _sensorData.temp = (buffer[3] << 12) | (buffer[4] << 4) | ( buffer[5] >> 4 );
    _sensorData.hum = (buffer[6] << 8) | buffer[7];

    BME280_DEBUG("T:0x%X  P:0x%X  H:0x%X",_sensorData.temp,_sensorData.pres,_sensorData.hum);
}


float  bme280_get_humidity()
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

    float humidity = (float)(var1>>12) / 1024.0;
    BME280_DEBUG("Humidity %.2f",humidity);
    return (float)humidity;    
}

float bme280_get_pressure()
{
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

    p = 1048576 - _sensorData.pres;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)_bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)_bme280_calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)_bme280_calib.dig_P7) << 4);

    //p is a pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits)
    float pressure = (float)p / 256; 
    BME280_DEBUG("Pressure %.2f",pressure);
    return pressure / 100; //Convert to hPa 
}

float bme280_get_temperature(){

  sensor_bust_read();

  int32_t var1, var2;

  var1 = (_sensorData.temp / 16384.0 - _bme280_calib.dig_T1 / 1024.0) * _bme280_calib.dig_T2;
  var2 = _sensorData.temp / 131072.0 - _bme280_calib.dig_T1 / 8192.0;
  var2 = var2 * var2 * _bme280_calib.dig_T3;

  /*
  var1  = ((((_sensorData.temp>>3) - ((int32_t)_bme280_calib.dig_T1 <<1))) * 
          ((int32_t)_bme280_calib.dig_T2)) >> 11;
  var2  = (((((_sensorData.temp>>4) - ((int32_t)_bme280_calib.dig_T1)) *
	        ((_sensorData.temp>>4) - ((int32_t)_bme280_calib.dig_T1))) >> 12) *
	        ((int32_t)_bme280_calib.dig_T3)) >> 14;
  */

  t_fine = var1 + var2;
  float temp  = t_fine / 5120.0; //(t_fine * 5 + 128) >> 8;
  BME280_DEBUG("Temp %.2f",temp);
  return temp;
}

#ifndef UNIT_TEST
#define SENSOR_START        bme280_sensor_start
#define SENSOR_GET_PRESSURE bme280_get_pressure
#define SENSOR_GET_HUMIDITY bme280_get_humidity
#define SENSOR_GET_TEMPERATURE bme280_get_temperature
#endif
