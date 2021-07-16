# WeatherStation-firmware-esp
Firmware for WeatherStation ESP8266


![Build status](https://github.com/vt77/WeatherStation-firmware-esp/actions/workflows/main.yml/badge.svg) 
![Version](https://img.shields.io/github/v/release/vt77/WeatherStation-firmware-esp)
![GitHub issues](https://img.shields.io/github/issues-raw/vt77/WeatherStation-firmware-esp)

## Introduction
This project is ESP8266 WiFi SoC firmware for WeatherStation. 

Although it requires an Arduino environment it does not use any public Arduino libraries and build mostly on arduino espressif8266 core framework. 

This is totally FREE software for personal use. You can modify it for your own purposes without any permissions from my side.

### Features
* CaptivePortal for AP settings and configuration
* Configurable webhook timer
* TLS webhook ( minimum of 1mb memory recommended )
* AS5600 magnetic rotary position sensor
* BME/BMP280 temperature/pressure/humidity sensor (not completely tested)
* SHT21 temperature/humidity sensor

### Supported Hardware

* ESP01
* Wemos D1 Mini
* ESP NodeMCU

## Static configuration

Before building firmware you can define your initial preferences. It can be changed later using the device web interface.

Example preferences found in *src/preferences.h.local*. Copy this file to your own *src/preferences.h*

```
cp src/preferences.h.local src/preferences.h
```

Open *src/preferences.h* in your favorit editor and change file according to your configuration.

DEVICE_ID - (string) your device id. Usually MAC address or any other for your own usage.

REV_PER_METER - (float) revolutions per meter of your anemometer.

SEND_INTERVAL - (integer) Interval to send webhook in seconds

TOKEN  - (string,optional) Your token. Will be sent in Authorization (or Access-Token ) header.

FINGERPRINT - (string) Webhook server TLS fingerprint 

For ESP01 with 512k flash (not recommended) you can compile firmware without CaptivePortal (see _ENABLE_HTTP_SERVER flag). So your WiFi settings MUST be hardcoded in flash. Following defines should hold real WiFi AP credentials 

WIFI_SSID  - WiFi AP name

WIFI_PASSWORD - WiFi AP password

## Select sensors

Currently only multiply temperature sensors may be selected. Define variable **TEMPERATURE_SENSOR** for one of the following values :

* SHT21 - (default) uses SHT21 sensor board
* BME280 - supports bmp280/bme280 . Type of device recognized by firmware 
* AHT20 - aht20 sensor board   

## Select datasender


### Sending data to narodmon 

Set DATASENDER to "NARODMON". This is default option.

```
DATASENDER=NARODMON
```

**Please note :**

Variable __NARODMON_USER__  should be set to your login,email or phonenumber of narodmon.ru for automatic station registration

```
NARODMON_USER="conion"
```

Optonal variable NARODMON_DEVICENAME may be set to name your device (default WeatherStation)

### Using webhook 

This option for advanced users. You can receive meteo data on your own server by http(s) webhook 

```
DATASENDER=WEBHOOK
```


## Building and uploading

```
make && make upload
```

**Please note :**
* Your ESP module should be connected to /dev/ttyUSB0 . Otherwise change your port in Makefile
* ESP-01 should be in programming mode
* platformio should be installed and found in path

## Custom build
You can customize your build by defining build flags. Following flags may be defined in *platformio.ini*

AUTH_FINGERPRINT  - use TLS fingerprint for server authentication

WEBHOOK_INSECURE - skip TLS server authentication (For debug purposes. Not recommended in production)

WEBHOOK_TOKEN_ENCRYPT - (default off) Encrypt web token . See [*token encryption*](#token-encryption). Useful for HTTP webhooks. 

_ENABLE_HTTP_SERVER - (default on) Disable local HTTP server. Useful for esp01 with 512k flash.

_ENABLE_TLS_WEBHOOK - (default on) Disable TLS(HTTPS). Saves about 200k flash size. (not recommended)

## Token encryption

With unencrypted connection (HTTP) or connection without server authentication 
(WEBHOOK_INSECURE) there is MITM attack (or even simple sniffing) possible allowing compromisation of security token. To minimize unauthorized access risks you can decide to enable token encryption.

When token encryption enabled the device will encrypt token before sending it in webhook header.The encryption procedure as following: 

1. Calculate date string from current year, day of the year, hour and minute
```        
        sprintf(datesting,"%d%d%d%d",
            timeinfo->tm_year + 1900,
            timeinfo->tm_yday,
            timeinfo->tm_hour,
            timeinfo->tm_min
        )
```
2. Use MD5 hash algorithm to encrypt original token with date

```
    totp_token =  md5(access_token+datesting)
```

So we get some kind of *Time-Based One Time Password*  valid for one minute and not vulnerable for md5 rainbow tables. It's still not good choice and **TLS encryption strongly recommended**. 

## Debugging

Some system messages sent to *Serial* port while the device running. You can configure output to be more verbose or even debug per unit (module). 

See *preferences.h* for more info

## License
This software is licensed under the MIT License. See LICENSE file for details

