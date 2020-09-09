# LWPubSub
LightWeight, energy-efficient, and end-to-end secure Publish-Subscribe system based on MQTT (LWPubSub)

Application code for the paper submitted to the Journal of Network and Computer Applications

This repository contains:
  - LWPubSub application for Constrained Wireless IoT devices (CWIoT)
  - LWPubSub IoT Agent (based on FIWARE IoT Agent for MQTT)
  - FIWARE-based cloud platform

## Preliminares:
  * Install docker and docker-compose
  * Create the container images:
```
cd cloud_platform; ./cloud-platform-services create
```

## Running a basic example
* Using 'terminator' (on Ubuntu: apt-get install terminator):
	- Open 5 "sub-terminals":
		- 1st for cloud-platform commands
			- Inside 'cloud_platform/' directory, execute: ``` ./cloud-platform-services start ```
		- 2nd for mosquitto_sub (view all specific MQTT messages)
			- Execute: ``` mosquitto_sub -h test.mosquitto.org -t '/99/#' -v ```
		- 3th on Raspberry Pi IoT device execute (inside 'LWPubSub_Rasp_application/' directory)
			- Previously it is necessary to provision the deviceID on the cloud platform (you can modify the clientID variable to use our value on the tests: 0012eb00f6d0 or 0012e3034d8c)
			- Execute: ```python lwpubsub-standalone.py CBC 256 test.mosquitto.org ```
		- 4th to check delivered MQTT messages to the cloud platform
			```
		    curl -X GET 'http://localhost:1026/v2/entities/0012eb00f6d0?type=Raspberry&options=keyValues' -H 'fiware-service: school' -H 'fiware-servicepath: /fd00' | python -m json.tool
		    ```
