
#include "JsonStaticString.h"

//1 min
#define MAX_TTL 60 * 1000L 

//WARNING !!! The insertion status not checked !. 
//Please provide enough space to hold all sensor's names and values
#define MAX_JSON_STRING_LENGTH  128

namespace vt77 {

static JsonStaticString<MAX_JSON_STRING_LENGTH> output;

class Sensor
{
    private:
        const char *n;
        const int p;
        double value;
        unsigned long last_updated;
    public:
        Sensor(const char *_name, int _precision = 2) : n(_name), p(_precision) {
        }

        const char * name() const {
            return n;
        };
        
        const int precision() const {
            return p;
        };

        void set(double _value)
        {
            value = _value;
            last_updated = millis();
        }

        double get() const 
        {   
            return value;
        }

        bool avaliable() const
        {
            return (bool)((millis() - last_updated) < MAX_TTL);
        }
};

template <typename T, unsigned S>
class SensorCollection
{
    private:
        const char *name;
        const T *sensors;
    public:
        SensorCollection(const char * _name, const T* _sensors):
                            name{_name}, sensors{_sensors} {};
        operator const char*()const {
            output.start();
            output.insert("id",name);
            for(unsigned int i=0;i<S;i++)
            {
                if(sensors[i].avaliable())
                    output.insert(sensors[i].name(),(const double)sensors[i].get(),sensors[i].precision());
            }
            output.close();
            return (const char *)output;
        };
};

}