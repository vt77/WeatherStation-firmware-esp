#include <unity.h>
#include <iostream>
#include <stdio.h>
#include "ArduinoMockup.h"


//Defined in main.cpp
#define SENSOR_TEMPERATURE    1
#define SENSOR_HUMIDITY       2

#define SHT21_DEBUG(fmt,...) std::printf("[SHT21][DEBUG]" fmt "\n", ## __VA_ARGS__)
#define SHT21_ERROR(fmt,...) std::printf("[SHT21][ERROR]" fmt "\n", ## __VA_ARGS__)

#include "sht21.h"

void test_sht21_sensor_start(void) {  
    twi_writeTo = [](unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) -> uint8_t{
        TEST_ASSERT_EQUAL_INT8_MESSAGE(I2C_SHT21_ADDRESS,address,"[TEST][SHT21]Bad sensor address");

        if(buf[0] == COMMAND_SOFT_RESET)
        {
            std::cout << "[SHT21][SENSORRESET]" << std::endl;
            TEST_ASSERT_EQUAL_INT8_MESSAGE(1,len,"Wrong reset data size");
        }else if (buf[0] == COMMAND_SETTINGS)
        {
            std::cout << "[SHT21][SENSORSETTINGS] 0x" << std::hex <<  (unsigned char)buf[1] << std::endl;
            TEST_ASSERT_EQUAL_INT8_MESSAGE(2,len,"Wrong settings data size");
        }else{
            TEST_FAIL_MESSAGE("[SHT21]Unknown command on init");
        }
        return 0;
    };
    
    uint8_t res = sht21_sensor_start();
    TEST_ASSERT_EQUAL_INT8_MESSAGE(3,res,"[TEST][SHT21]Bad sensor start return");
}

void test_sht21_get_temperature(void) {

    twi_writeTo = [](unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) -> uint8_t{
            TEST_ASSERT_EQUAL_INT8_MESSAGE(I2C_SHT21_ADDRESS,address,"[TEST][SHT21]Bad sensor address");
            TEST_ASSERT_EQUAL_INT8_MESSAGE(COMMAND_TEMPERATURE,buf[0],"[TEST][SHT21]Read temperature command");
            return 0;
    };

    twi_readFrom = [](unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) -> uint8_t
    {
        //Packet format [temperature1,temperature2,crc]
        TEST_ASSERT_EQUAL_INT8_MESSAGE(I2C_SHT21_ADDRESS,address,"[TEST][SHT21]Bad sensor address");
        TEST_ASSERT_EQUAL_INT8_MESSAGE(3,len,"[TEST][SHT21]Bad buffer length");
        buf[0] = 0x55;
        buf[1] = 0x00;
        buf[2] = calc_crc(buf[0] << 8 | buf[1]);
        return 0;
    };

    float temp = sht21_get_temperature();
    TEST_ASSERT_EQUAL_INT16_MESSAGE(1149,(unsigned int)(temp*100),"[TEST][SHT21]Bad get_temperature return value");
}


void test_sht21_get_humidity(void) {
    twi_writeTo = [](unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) -> uint8_t{
            TEST_ASSERT_EQUAL_INT8_MESSAGE(I2C_SHT21_ADDRESS,address,"[TEST][SHT21]Bad sensor address");
            TEST_ASSERT_EQUAL_INT8_MESSAGE(COMMAND_HUMIDITY,buf[0],"[TEST][SHT21]Read temperature command");
            return 0;
    };

    twi_readFrom = [](unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) -> uint8_t
    {
        //Packet format [hum11,hum2,crc]
        TEST_ASSERT_EQUAL_INT8_MESSAGE(I2C_SHT21_ADDRESS,address,"[TEST][SHT21]Bad sensor address");
        TEST_ASSERT_EQUAL_INT8_MESSAGE(3,len,"[TEST][SHT21]Bad buffer length");
        buf[0] = 0x65;
        buf[1] = 0x04;
        buf[2] = calc_crc(buf[0] << 8 | buf[1]);
        return 0;
    };

    float hum = sht21_get_humidity();
    TEST_ASSERT_EQUAL_INT16_MESSAGE(4331,(unsigned int)(hum*100),"[TEST][SHT21]Bad get_humidity return value");
}


void sht21_test(){
    UNITY_BEGIN();
    RUN_TEST(test_sht21_sensor_start);
    RUN_TEST(test_sht21_get_temperature);
    RUN_TEST(test_sht21_get_humidity);
    UNITY_END();
}

#define TEST_SHT21 sht21_test


