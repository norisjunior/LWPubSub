#!/bin/bash

echo "#############################################################"
echo "###        Please, check if MQTT Broker is working        ###"
echo "PERSISTENCE - CYGNUS! - CHECK "
echo "#############################################################"

sleep 1;







echo "#############################################################"
echo "###       CWIoT provisioning on FIWARE and IoT Agent      ###"
echo "#############################################################"

echo "###### API-KEY in this environment: 99"

###########################################################################
####### API-KEY ###########################################################
###########################################################################
curl -iX POST \
  'http://localhost:4041/iot/services' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "services": [
   {
     "apikey":      "99",
     "cbroker":     "http://orion:1026",
     "entity_type": "LWPUBSUB",
     "resource":    "/iot/d"
   }
 ]
}'


sleep 1;





###########################################################################
####### Device provisioning ###############################################
###########################################################################

###########################################################################
######    CWIoT suffix: 010203060708 (Native)                       #######
###########################################################################

echo "\n###### CWIoT provisioning on the cloud platform:\n"

echo "\n###### NATIVE TARGET CWIoT\n"

curl -iX POST \
  'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "devices": [
   {
     "device_id":   "010203060708",
     "entity_name": "010203060708",
     "entity_type": "Native",
     "protocol":    "PDI-IoTA-UltraLight",
     "transport":   "MQTT",
     "timezone":    "Europe/Berlin",
     "commands": [
       {"name": "33030","type": "command"},
       {"name": "33040","type": "command"},
       {"name": "33110","type": "command"},
       {"name": "33111","type": "command"},
       {"name": "33380","type": "command"}
      ],
      "attributes": [
       {"object_id": "33030", "name": "33030", "type": "actual_temp"},
       {"object_id": "33040", "name": "33040", "type": "actual_hum"}
      ],
      "static_attributes": [
        {"name":"refStore", "type": "Relationship","value": "urn:ngsi-ld:School:001"}
     ]
   }
 ]
}
'

sleep 1;


###########################################################################
######    CWIoT suffix: 00124b05257a (Native)                       #######
###########################################################################

echo "\n###### SENSORTAG CC2650 TARGET CWIoT\n"

curl -iX POST \
  'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "devices": [
   {
     "device_id":   "00124b05257a",
     "entity_name": "00124b05257a",
     "entity_type": "Sensortag",
     "protocol":    "PDI-IoTA-UltraLight",
     "transport":   "MQTT",
     "timezone":    "Europe/Berlin",
     "commands": [
       {"name": "33030","type": "command"},
       {"name": "33040","type": "command"},
       {"name": "33110","type": "command"},
       {"name": "33111","type": "command"},
       {"name": "33380","type": "command"}
      ],
      "attributes": [
       {"object_id": "33030", "name": "33030", "type": "actual_temp"},
       {"object_id": "33040", "name": "33040", "type": "actual_hum"}
      ],
      "static_attributes": [
        {"name":"refStore", "type": "Relationship","value": "urn:ngsi-ld:School:001"}
     ]
   }
 ]
}
'


sleep 1;



###########################################################################
######    CWIoT suffix: 00124b4a527d (Native)                       #######
###########################################################################

echo "\n###### REMOTE CC2538 TARGET CWIoT\n"

curl -iX POST \
  'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "devices": [
   {
     "device_id":   "00124b4a527d",
     "entity_name": "00124b4a527d",
     "entity_type": "Remote",
     "protocol":    "PDI-IoTA-UltraLight",
     "transport":   "MQTT",
     "timezone":    "Europe/Berlin",
     "commands": [
       {"name": "33030","type": "command"},
       {"name": "33040","type": "command"},
       {"name": "33110","type": "command"},
       {"name": "33111","type": "command"},
       {"name": "33380","type": "command"}
      ],
      "attributes": [
       {"object_id": "33030", "name": "33030", "type": "actual_temp"},
       {"object_id": "33040", "name": "33040", "type": "actual_hum"}
      ],
      "static_attributes": [
        {"name":"refStore", "type": "Relationship","value": "urn:ngsi-ld:School:001"}
     ]
   }
 ]
}
'


sleep 1;


echo "############################################################"
echo "############                                    ############"
echo "###########                                      ###########"
echo "##########          RESUMED DEVICES INFO          ##########"
echo "###########                                      ###########"
echo "############                                    ############"
echo "############################################################"


sleep 1;

curl -X GET \
  'http://localhost:1026/v2/entities/010203060708?type=Native&options=keyValues' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' | python -m json.tool

sleep 1;

curl -X GET \
  'http://localhost:1026/v2/entities/00124b05257a?type=Sensortag&options=keyValues' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' | python -m json.tool

sleep 1;


curl -X GET \
  'http://localhost:1026/v2/entities/00124b4a527d?type=Remote&options=keyValues' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' | python -m json.tool

sleep 1;





echo "############################################################"
echo "############                                    ############"
echo "###########                                      ###########"
echo "##########              SEND COMMAND!             ##########"
echo "###########                                      ###########"
echo "############                                    ############"
echo "############################################################"

sleep 1;

echo -e "Turn on RED LED:"
curl -iX POST \
  http://localhost:4041/v2/op/update \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
    "actionType": "update",
    "entities": [
        {
            "type": "Sensortag",
            "id": "00124b05257a",
            "33110" : {
                "type": "command",
                "value": "1"
            }
        }
    ]
}'




echo -e "Turn on GREEN LED:"
curl -iX POST \
  http://localhost:4041/v2/op/update \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
    "actionType": "update",
    "entities": [
        {
            "type": "Sensortag",
            "id": "00124b05257a",
            "33111" : {
                "type": "command",
                "value": "1"
            }
        }
    ]
}'





echo -e "Request temperature measurement:"
curl -iX POST \
  http://localhost:4041/v2/op/update \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
    "actionType": "update",
    "entities": [
        {
            "type": "Sensortag",
            "id": "00124b05257a",
            "33030" : {
                "type": "command",
                "value": "0"
            }
        }
    ]
}'
