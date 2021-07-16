#ifndef NARODMON_DEBUG
#define NARODMON_DEBUG(...)
#endif

#ifndef NARODMON_ERROR
#define NARODMON_ERROR(...)
#endif

#include "DataSender.hpp"

#define MAX_BUFFER_STRING_LENGTH 128

namespace vt77 {

    /**
     *  Narodmon protocol handler
     * 
     *  It's a simple ascii protocol 
     *  Sends one chank of data per line 
     * 
     *  #XX-XX-XX-XX-XX-XX#WeatherStation -  Id and name (mandatory) 
     *  #OWNER#your_name                  -  Owner. May be sended once 
     *  #LAT#XX.XXXXX                     -  Device location. Sent once
     *  #LON#XX.XXXXX                       
     *  //Sensor data . Multiply lines may be sent
     *  #SensorId#SensorValue#ReadableName 
     * 
     *  ##  - End of packet
     */

    class NarodmonProto
    {
        public:
            void start(const char *id)
            {
                sprintf(buffer,"#%s#" NARODMON_DEVICENAME " \n",id);
                #ifdef NARODMON_OWNER
                strcat(buffer, "#OWNER#" NARODMON_OWNER "\n");
                #endif
            }

            void close()
            {
                strcat(buffer,"##");
            }

            bool insert(const char * key, double value, int precision=2){
                NARODMON_DEBUG("Adding %s : %.2f",key,value);
                char format[16];
                char entry[32];
                sprintf(format,"#%%s#%%.%df\n",precision);
                sprintf(entry,format,key,value);
                strcat(buffer,entry);
                if(strlen(buffer) + strlen(entry) >= MAX_BUFFER_STRING_LENGTH)
                        return false;
                return true;
            }

            operator const char*()const {
                return buffer;
            }
        private:
            static char buffer[];
    };

    class NarodmonSender : public DataSender
    {
        public:
            void init(const char * dsn) override{
                NARODMON_DEBUG("Init : %s",dsn);
            };

            void send_data(const char * access_token, const char * body) override{
                WiFiClient client;
                NARODMON_DEBUG("Connecting to narodmon");
                if( !client.connect("narodmon.ru", 8283) )
                {
                    NARODMON_ERROR("Can't connect to narodmon");
                    return;
                }
                NARODMON_DEBUG("Send %s" , body);
                client.print(body); // Send

                /*
                //WARNING !!! String class uses dynamic memory allocation and may cause memory fragmentation
                //Use it for debug purposes only if you want to see full server responce
                while (client.available()) {
                    String line = client.readStringUntil('\n');
                    NARODMON_DEBUG("Response : %s",line.c_str() );
                    if(!line.startsWith("OK"))
                    {
                        NARODMON_ERROR("Server error : %s",line.c_str() );    
                    }
                }
                */

                char resp[16];
                client.read(resp,16);
                NARODMON_DEBUG("Response : %s",resp);
                if( strncmp("OK",resp,2) != 0 )
                {
                    NARODMON_ERROR("Server error : %s", resp);
                }
            };
    };

    char NarodmonProto::buffer[MAX_BUFFER_STRING_LENGTH];
    #define DATA_FORMAT NarodmonProto
    NarodmonSender datasender;
}