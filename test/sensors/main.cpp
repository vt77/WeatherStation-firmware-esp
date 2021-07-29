#include <unity.h>
#include<iostream>
#include <stdio.h>


#define SENSOR_WINDSPEED      0
#define SENSOR_TEMPERATURE    1
#define SENSOR_HUMIDITY       2
#define SENSOR_WINDDIR        3
#define SENSOR_PRESSURE       4


#include "counter.hpp"
#include "sht21.hpp"
#include "bme280.hpp"


int main()
{
    TEST_COUNTER();
    TEST_SHT21();
    TEST_BME280();
}
