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

## Running a basic example with native Contiki-NG target (simulated IoT device)
* Using 'terminator' (on Ubuntu: apt-get install terminator):
	- Open 5 "sub-terminals":
		- 1st for cloud-platform commands (start and provision device)
			- Inside 'cloud_platform/' directory, execute: ``` ./cloud-platform-services start ```
      - After, run: ``` ./new-provisioning.sh ``` to provision the CWIoT (Sensortag, Remote, Native)
		- 2nd for local MQTT Broker - use mosquitto_sub (view all specific MQTT messages)
			- Execute: ``` mosquitto_sub -t "#" -v ```
		- 3th to check IoT Agent messages:
			- Execute: ``` ./view_logs.sh ```
    - 4th to run the device:
      - Native target simulates the CWIoT. It is required to insert the ``` lwpubsub_system ``` inside the ``` examples/ ``` folder of Contiki-NG (we use the version 4.4)
      - After installing Contiki-NG, we need to compile the code, running ``` ./make_lwpubsub_native-cloud_cwiot.sh ```
      - Then execute: ``` sudo ./lwsec-pubsub-system.native ```
		- 5th to check delivered MQTT messages to the cloud platform
			```
		    curl -X GET 'http://localhost:1026/v2/entities/010203060708?type=Native&options=keyValues' -H 'fiware-service: school' -H 'fiware-servicepath: /fd00' | python -m json.tool
		    ```
