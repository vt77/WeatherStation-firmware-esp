#include <unity.h>
#include <iostream>
#include <stdio.h>
#include "ArduinoMockup.h"
#include "stdint.h"

#define BME280_DEBUG(fmt,...) std::printf("[BMP280][DEBUG]" fmt "\n", ## __VA_ARGS__)
#define BME280_ERROR(fmt,...) std::printf("[BMP280][ERROR]" fmt "\n", ## __VA_ARGS__)


#include "bme280.h"


/*

[BME280][DEBUG][CALIB][T1] 26481
[BME280][DEBUG][CALIB][T2] 26313
[BME280][DEBUG][CALIB][T3] -1000
[BME280][DEBUG][CALIB][P1] 36332
[BME280][DEBUG][CALIB][P2] -10586
[BME280][DEBUG][CALIB][P3] 3024
[BME280][DEBUG][CALIB][P4] 2815
[BME280][DEBUG][CALIB][P5] 168
[BME280][DEBUG][CALIB][P6] -7
[BME280][DEBUG][CALIB][P7] 15500
[BME280][DEBUG][CALIB][P8] -14600
[BME280][DEBUG][CALIB][P9] 6000



0x7A930  P:0x6C990  H:0x924C

*/



uint16_t calibration_data[] = { 
       // 28009,  //dig_t1 
       // 25654,  //dig_t2 
       // 50,     //dig_t3 
        26481,
        26313,
        (uint16_t)-1000,
       
        /*
        39145,  //dig_p1 
        (uint16_t)-10750,  //dig_p2
        3024,    //dig_p3
        5667,    //dig_p4
        (uint16_t)-120,    //dig_p5
        (uint16_t)-7,      //dig_p6
        15500,   //dig_p7
        (uint16_t)-14600,  //dig_p8
        6000,    //dig_p9
        */
        
        36332,
        (uint16_t)-10586,
        3024,
        2815,
        168,
        (uint16_t)-7,
        15500,
        (uint16_t)-14600,
        6000
        
};

uint8_t hum_calibration_data[]  = {
    //TODO: Find real data for tests
    0,0,0,0,0,0,0
};


//#define TEST_TEMP_RAW 529191
//#define TEST_TEMP_CMP 24.79
#define TEST_TEMP_RAW 0x7B946
#define TEST_TEMP_CMP 25.87383

//#define TEST_PRES_RAW 326816
//#define TEST_PRES_CMP 1006.61517564
#define TEST_PRES_RAW  0x6C990
#define TEST_PRES_CMP 961.4433


#define TEST_ALT_CMP 57.3174

//3 bytes pressure, 3 bytes temp , 2 bytes hum
uint8_t bme280_sensor_data[] = {
    (TEST_PRES_RAW & 0xFF000) >> 12, (TEST_PRES_RAW & 0x00FF0) >> 4, (TEST_PRES_RAW & 0x0000F) << 4,
    (TEST_TEMP_RAW & 0xFF000) >> 12, (TEST_TEMP_RAW & 0x00FF0) >> 4, (TEST_TEMP_RAW & 0x0000F) << 4,
    0x92,0x4C //This values sent by BMP280
};


uint8_t bme280_send_chip_id(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop)
{
        TEST_ASSERT_EQUAL_INT8_MESSAGE(BME280_SENSOR_ADDRESS,address,"[TEST][BME280]Bad sensor address");
        TEST_ASSERT_EQUAL_INT8_MESSAGE(1,len,"[TEST][BME280]Bad buffer length");
        buf[0] = 0x60;
        return 0;
}

uint8_t bme280_send_calibration1(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop)
{
        TEST_ASSERT_EQUAL_INT8_MESSAGE(BME280_SENSOR_ADDRESS,address,"[TEST][BME280]Bad sensor address");
        TEST_ASSERT_EQUAL_INT8_MESSAGE(26,len,"[TEST][BME280]Bad buffer length");
        for(int i=0;i<13;i++)
        {
            buf[i*2] = calibration_data[i] & 0xff;
            buf[i*2+1] = calibration_data[i] >> 8;
        }
        return 0;
}

uint8_t bme280_send_calibration2(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop)
{
        TEST_ASSERT_EQUAL_INT8_MESSAGE(BME280_SENSOR_ADDRESS,address,"[TEST][BME280]Bad sensor address");
        TEST_ASSERT_EQUAL_INT8_MESSAGE(7,len,"[TEST][BME280]Bad buffer length");
        for(int i=0;i<7;i++)
        {
            buf[i] = hum_calibration_data[i] & 0xff;
        }
        return 0;
}


uint8_t bme280_send_data(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop)
{
        TEST_ASSERT_EQUAL_INT8_MESSAGE(BME280_SENSOR_ADDRESS,address,"[TEST][BME280]Bad sensor address");
        TEST_ASSERT_EQUAL_INT8_MESSAGE(8,len,"[TEST][BME280]Bad buffer length");
        for(int i=0;i<8;i++)
        {
            buf[i] = bme280_sensor_data[i] & 0xff;
        }
        return 0;
}


void test_bme280_sensor_start()
{
    uint8_t res = bme280_sensor_start();
    TEST_ASSERT_EQUAL_INT8_MESSAGE(SENSOR_TEMPERATURE|SENSOR_HUMIDITY|SENSOR_PRESSURE,res,"[TEST][BME280]Bad sensor address");
}

void test_bme280_get_temperature()
{
    float temp = bme280_get_temperature();
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(TEST_TEMP_CMP,temp,"[TEST][BME280]Wrong temperature value");
}

void test_bme280_get_humidity()
{
    //TODO: Not tested yet
    float humidity = bme280_get_humidity();
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0,humidity,"[TEST][BME280]Wrong humidity value");
}

void test_bme280_get_pressure()
{
    float pressure = bme280_get_pressure();
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(TEST_PRES_CMP,pressure,"[TEST][BME280]Wrong pressure value");
}

void bme280_test(){

   twi_writeTo = [](unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) -> uint8_t{
            TEST_ASSERT_EQUAL_INT8_MESSAGE(BME280_SENSOR_ADDRESS,address,"[TEST][BME280]Bad sensor address");
            std::cout << "[BMP280][COMMAND]LEN: " << len << std::endl;
            std::cout << "[BMP280][COMMAND] " << std::hex <<  (unsigned char)buf[0] << std::endl;
            switch(buf[0]){
                case BME280_REG_ID:
                    twi_readFrom = bme280_send_chip_id;
                    break;

                case 0x88: //Start of calibration data
                    twi_readFrom = bme280_send_calibration1;
                    break;

                case 0xE1: //Humidity calibration data 
                    twi_readFrom = bme280_send_calibration2;
                    break;

                case BME280_REGISTER_CONFIG:
                case BME280_REGISTER_CONTROL_HUM:
                case BME280_REGISTER_CONTROL:
                    //TODO: Check possible values
                    break;

                case 0xF7: //Start of temperture/pressure/humidity data
                    twi_readFrom = bme280_send_data;
                    break;

                default:
                    TEST_FAIL_MESSAGE("Unknown command");
                    break;
            }
            return 0;
    };

    UNITY_BEGIN();
    RUN_TEST(test_bme280_sensor_start);
    RUN_TEST(test_bme280_get_temperature);
    RUN_TEST(test_bme280_get_humidity);
    RUN_TEST(test_bme280_get_pressure);
    UNITY_END();
}

#define TEST_BME280 bme280_test

