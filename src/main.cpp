//Static preferences (defaults)
#include "preferences.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <FS.h>
#include <LittleFS.h>
#include <Preferences.hpp>

#ifdef _ENABLE_HTTP_SERVER
#include <CaptivePortal.h>
#endif

#include "JsonStaticString.h"


#ifdef DATASENDER
#include DATASENDER
#endif


#define SENSOR_WINDSPEED      0
#define SENSOR_TEMPERATURE    1
#define SENSOR_HUMIDITY       2
#define SENSOR_WINDDIR        3
#define SENSOR_PRESSURE       4

#include "sensors.hpp"
#include "tasker.hpp"

#include "twi.h"

#ifdef TEMPERATURE_SENSOR
#include TEMPERATURE_SENSOR
#endif

#include "counter.h"
#include "as5600.h"

#ifndef I2CSDA
#define I2CSDA 0
#endif
#ifndef I2CSCL
#define I2CSCL 2
#endif

using namespace vt77;

#include "utils.h"

WiFiClient net;
Tasker tasker;

#define SENSORS_COUNT  5
Sensor sensors[SENSORS_COUNT] = { 
        Sensor("speed",1),
        Sensor("temp"), 
        Sensor("hum",1),
        Sensor("dir",0),
        Sensor("press",1),
};
SensorCollection<Sensor,SENSORS_COUNT> sensorsCollection(DEVICE_ID,sensors);

typedef struct{
  float rev_per_meter;
  unsigned int interval;
  char token[16];
  char fingerprint[64];
  unsigned char north_offset;
}deviceconfig_t;

deviceconfig_t deviceconfig = {
    REV_PER_METER,
    SEND_INTERVAL,
    TOKEN,
    FINGERPRINT
};
Preferences<deviceconfig_t> preferences(&deviceconfig);

#ifdef _ENABLE_HTTP_SERVER
CaptivePortal captivePortal;
const get_preferences_t CaptivePortal::preferencesJson = []() -> String {
    JsonStaticString<256> json;
    json.start();
    json.insert("rev_per_meter",(double)deviceconfig.rev_per_meter);
    json.insert("token",deviceconfig.token);
    json.insert("fingerprint",deviceconfig.fingerprint);
    json.insert("interval",(int)deviceconfig.interval);
    json.insert("north_offset",(int)deviceconfig.north_offset);
    json.close();
    return String(json);
};
#endif

void setup() {

    Serial.begin(115200);
    WiFi.printDiag(Serial);

    if( WiFi.SSID().length() == 0 || WiFi.psk().length() == 0 )
    {
        Serial.println("Start captive portal");
        #ifdef _ENABLE_HTTP_SERVER
          captivePortal.start(MODE_LOCAL);
        #else
            WiFi.persistent(true);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //start station with new credentials
        #endif

        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin();
    WiFi.setAutoReconnect(true);
    //WiFi.setOutputPower(10);  // reduce RF output power, increase if it won't connect
    //WiFi.waitForConnectResult();
    #ifdef SLEEP_DELAY
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    #endif


    WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& status) {
        Serial.print("[WIFI]Connected, IP address: ");
        Serial.println(WiFi.localIP());
    });
    Serial.print("Connect to AP ");
    Serial.println(WiFi.SSID());
    
    wl_status_t status;
    while ( ( status = WiFi.status() ) != WL_CONNECTED)
    {
            if( status == WL_WRONG_PASSWORD || status == WL_CONNECT_FAILED )
            {
                Serial.print("Connection failed : 0x");
                Serial.println(status,HEX);
                #ifdef _ENABLE_HTTP_SERVER
                Serial.println("Start captive portal");
                captivePortal.start(MODE_LOCAL);
                #endif
            }
            Serial.print(".");
            delay(500);
    }

    Serial.println(" Done");
    Serial.print("[WIFI]Connected, IP address: ");
    Serial.println(WiFi.localIP());

    if (!LittleFS.begin()) {
      Serial.println("LittleFS mount failed");
      return;
    }

#ifdef _ENABLE_HTTP_SERVER
    Serial.println("[WEBSRV]Start");
    captivePortal.start(MODE_PUBLIC);
    captivePortal.onPreferences(std::bind([](deviceconfig_t *config){
        strncpy( config->fingerprint,captivePortal.get("fingerprint").c_str(),64);
        strncpy( config->token,captivePortal.get("token").c_str(),16);
        config->interval = captivePortal.get("interval").toInt();
        config->north_offset = captivePortal.get("north_offset").toInt();
        config->rev_per_meter = captivePortal.get("rev_per_meter").toFloat();
        Serial.print("[WEBSRV]Save preferences ");
        Serial.println(CaptivePortal::preferencesJson());
        preferences.save();
    },&deviceconfig));
#endif

    if(!preferences.load())
    {
        Serial.println("Error load preferences. Using default");
    }

    datasender.init(deviceconfig.fingerprint);

    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    Serial.println("Start I2C devices");
    twi_init(I2CSDA,I2CSCL);
    twi_setClock(10000L); //10kHz for PIC12 counter
    int flags = SENSOR_START();
    Serial.print("Temperature sensor flags : ");
    Serial.println(flags, HEX);
    counter_init();
    if( counter_get_last_error() != 0 )
    {
        Serial.println("WINDSPEED sensor not present");
        goto skip_windspeed;
    }

    //Calculte wind speed each second
    tasker.attach(1,std::bind([](Sensor *windspeed){
            static double runningAvg = 0;
            unsigned int cnt = counter_read_diff();
            if(counter_get_last_error() != counter_error_t::ERROR_OK )
            {
                runningAvg = 0;
                return;
            }

            float velocity = cnt / deviceconfig.rev_per_meter;
            if(velocity > 50)
                //Really?  100 knots is it too much ?! .
                return;

            //Simple integrator for smooth avarage
            if(runningAvg == 0)
                runningAvg = velocity;
            runningAvg = ( runningAvg + velocity ) / 2;
            sensors[SENSOR_WINDSPEED].set(runningAvg);
            Serial.print("[WINDSPEED]");
            Serial.println(sensors[SENSOR_WINDSPEED].get());
        },&sensors[SENSOR_WINDSPEED]
    ));
skip_windspeed:

    //Read sensors and send webhook
    tasker.attach(deviceconfig.interval ,std::bind([flags](){
            float temp = SENSOR_GET_TEMPERATURE();
            if(temp != VALUE_ERROR)
            {
                //Temperature is a basis for calculation pressure and humidity 
                //Should be raded first

                sensors[SENSOR_TEMPERATURE].set( temp );
                Serial.print("[TEMP]");
                Serial.println(sensors[SENSOR_TEMPERATURE].get());

                if(flags & SENSOR_HUMIDITY)
                {
                  sensors[SENSOR_HUMIDITY].set( SENSOR_GET_HUMIDITY());
                  Serial.print("[HUMIDITY]");
                  Serial.println(sensors[SENSOR_PRESSURE].get());
                }

                if(flags & SENSOR_PRESSURE)
                {
                  float p = SENSOR_GET_PRESSURE();
                  sensors[SENSOR_PRESSURE].set((unsigned int)to_mmHg_at_sealevel(p,737)); //In mmHg
                  Serial.print("[PRESSURE]");
                  Serial.println(sensors[SENSOR_PRESSURE].get());
                }

            }else{
                  Serial.println("[ERROR]Sensor not ready");
            }

            unsigned char winddir = read_direction();
            if(winddir != DIRECTION_DEVICE_ERROR)
            {
              sensors[SENSOR_WINDDIR].set(( winddir + deviceconfig.north_offset ) & 0x0F); //16 cardinal points 
              Serial.print("[WINDDIR]");
              Serial.println(sensors[SENSOR_WINDDIR].get());
            }

            const char * data = static_cast<const char*>(sensorsCollection);
            Serial.print("[SEND]");
            Serial.println(data);      
            datasender.send_data(deviceconfig.token,data);
            Serial.print("[FREEMEM] ");
            Serial.println(ESP.getFreeHeap());
                    
        }));
}

void loop() {
#ifdef _ENABLE_HTTP_SERVER  
    captivePortal.process();
#endif
    tasker.process();

/*
wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
wifi_fpm_open();
//Sleep a second
wifi_fpm_do_sleep(0xFFFFFFF); // only 0xFFFFFFF works; any other value and it won't sleep
WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 3);
WiFi.forceSleepWake();
delay(10);
*/

}