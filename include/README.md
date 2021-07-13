Sensors handlers
---

# Supported sensors

* [as5600](#as5600) 
* [sht21](#sht21)
* [bme280/bmp280](#bme280/bmp280)
* [aht20](#aht20)

## as5600
The AS5600 is an magnetic rotary position sensor with a high-​resolution 12-bit output. It supports two power modes - 3.3 and 5 volt (using internal LDO). I2C or PWM interface may be used to read position value.

We use AS5600 in windvane sensor to read wind direction position. Only 4 bits of value used, and it 
gives 16 actual positions, that's pretty enough for wind direction.

### Wiring diagram

Coming soon

## sht21
Sensor contains capacitive type humidity sensor, band gap temperature sensor and specialized analogue and digital integrated circuit on a single CMOSens chip. This yields an unmatched sensor performance in terms of accuracy and stability as well as minimal power consumption.

Supply voltage range from 2.1V to 3.6V (5v in module)

Temperature accuracy of ±0.3°C with up to 0.01°C (14bit) temperature resolution

Relative humidity accuracy of ±2%RH with up to 0.04%RH (12bit) resolution

### Wiring diagram

Coming soon


## bme280/bmp280

The BME280 is a humidity/temperature/pressure sensor especially developed for mobile applications and wearables where size and low power consumption are key design parameters. The unit combines high linearity and high accuracy sensors and is perfectly feasible for low current consumption and long-term stability.

BMP280 is the same as BME280 but doesn't have humidity measurement. The firmware can detect type of sensor automaticaly.  

Supply voltage range from 1.7V to 3.6V (5v in module)

Humidity accuracy of ±3%

Pressure accuracy of ±0.25%

Average current consumption in sleep/active 0.1 μA / 2.8 μA

### Wiring diagram

Coming soon


## aht20
Coming soon


