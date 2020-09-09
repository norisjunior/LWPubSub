CWIoT - MQTT
============

* The mqtt client can be used to connect with a MQTT Broker and 'publish' according to topics described in the article.
* To compile, at least three main variables need to be used: TARGET, MAKE_KEYSIZE, and MAKE_CRYPTOMODE
* In lsci-cwiot_v0.3.c one can set `#define LOG_LEVEL LOG_LEVEL_DBG` to see detailed crypto informations output from the console.
* A basic working example has an MQTT Broker in the address fd00::1/64 (can be a localhost mosquitto) and an instance of sea_cloud already running.

* To make the basic working example:
	- `make TARGET=native MAKE_KEYSIZE=128 MAKE_CRYPTOMODE=CBC`

* And then:
	- `sudo ./lsci-cwiot_v0.3.native`

* The default publish interval is 15 seconds