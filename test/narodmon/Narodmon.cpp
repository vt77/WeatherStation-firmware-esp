#include <unity.h>
#include<iostream>
#include <stdio.h>
#include "JsonStaticString.h"


class MockString : public std::string
{
    public:
        MockString(const char * s):std::string(s){}
        bool startsWith(const char *s)
        {
            return (std::string::rfind(s, 0) == 0);
        }
};

class MockWiFiClient
{
    public:

    int connect(const char * host, int port)
    {
            std::cout << "MockConnect to " <<  host << ":" << port << std::endl;
            return 1;
    }

    int print(const char * data )
    {
        //Check strings
        TEST_ASSERT_MESSAGE(strstr(data,"#H1#43.5") != 0,"H1 value wrong");
        TEST_ASSERT_MESSAGE(strstr(data,"#T1#20.52") != 0,"T1 value wrong");
        TEST_ASSERT_MESSAGE(strstr(data,"##") != 0,"End of packet missing");
        return 1;
    }

    MockString readStringUntil(char c)
    {
        MockString s = "OK";
        return s;
    }

    int read(char * buff, int len)
    {
        snprintf(buff,len,"OK\r\n");
        return 0;
    }
};



#define NARODMON_DEBUG(fmt,...) std::printf("[NARODMON][DEBUG]" fmt "\n", ## __VA_ARGS__)
#define NARODMON_ERROR(fmt,...) std::printf("[NARODMON][ERROR]" fmt "\n", ## __VA_ARGS__)

#define NARODMON_DEVICENAME "WeatherStation"
#define NARODMON_OWNER "test"

//Mock WiFiClient class
#define WiFiClient MockWiFiClient
#define String MockString

#include "narodmon.hpp"

vt77::DATA_FORMAT narodmonProto;

void test_narodmon(void) {

    narodmonProto.start("00:00:00:00:00");
    narodmonProto.insert("H1",43.5,1);
    narodmonProto.insert("T1",20.52,2);
    narodmonProto.close();
    vt77::datasender.send_data(0,(const char *)narodmonProto);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_narodmon);
    UNITY_END();
    std::cout << narodmonProto << std::endl;
}

