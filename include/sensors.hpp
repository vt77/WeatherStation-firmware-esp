
//1 min
#define MAX_TTL 60 * 1000L 

namespace vt77 {

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


DATA_FORMAT dataSenderBuffer;
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
            dataSenderBuffer.start(name);
            for(unsigned int i=0;i<S;i++)
            {
                if(sensors[i].avaliable())
                    dataSenderBuffer.insert(sensors[i].name(),(const double)sensors[i].get(),sensors[i].precision());
            }
            dataSenderBuffer.close();
            return (const char *)dataSenderBuffer;
        };
};

}