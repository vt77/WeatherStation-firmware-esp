
#include <stdint.h>
#include <unity.h>
#include "ArduinoMockup.h"

#include "counter.h"

#include <iomanip>


static std::string stage;

void test_counter_read_diff(void)
{

    stage = "INIT";
    counter_init();

    stage = "READ";
    uint16_t cnt = counter_read();
    TEST_ASSERT_EQUAL_INT16_MESSAGE(0,cnt,"[TEST][COUNTER][READ]Bad counter value");

    stage = "DIFF";
    //Check counter overflow
    uint16_t diff = counter_read_diff();
    TEST_ASSERT_EQUAL_INT16_MESSAGE(0x20,diff,"[TEST][COUNTER][DIFF]Bad difference value");
}



void counter_test()
{
    //Emulate overflow
    static uint16_t counter_value = 0xFFF0;
    twi_readFrom = [](unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) -> uint8_t
    {
        TEST_ASSERT_EQUAL_INT8_MESSAGE(I2C_COUNTER_ADDRESS,address,"[TEST][COUNTER]Bad sensor address");
        TEST_ASSERT_EQUAL_INT8_MESSAGE(2,len,"[TEST][COUNTER]Bad buffer size");
        std::cout << "[TEST][COUNTER][" <<  stage << "]Counter 0x"  << std::hex << std::setfill('0') << std::setw(4) << counter_value << std::endl;
        buf[0] = counter_value & 0xFF;
        buf[1] = counter_value >> 8;
        counter_value += 0x10;
        return 0;
    };

    UNITY_BEGIN();
    RUN_TEST(test_counter_read_diff);
    UNITY_END();
}


#define TEST_COUNTER counter_test