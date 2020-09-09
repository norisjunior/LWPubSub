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


echo "\n###### LAUNCHPAD1 CC2650 TARGET CWIoT\n"

curl -iX POST \
  'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "devices": [
   {
     "device_id":   "00124b82ad03",
     "entity_name": "00124b82ad03",
     "entity_type": "Launchpad",
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



echo "\n###### LAUNCHPAD2 CC2650 TARGET CWIoT\n"

curl -iX POST \
  'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "devices": [
   {
     "device_id":   "00124b82b206",
     "entity_name": "00124b82b206",
     "entity_type": "Launchpad",
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



echo "\n###### Windows WiFi standalone app\n"

curl -iX POST \
  'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "devices": [
   {
     "device_id":   "0012e3034d8c",
     "entity_name": "0012e3034d8c",
     "entity_type": "WindowsWiFi",
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



# Raspberry pi: 'b8:27:eb:00:f6:d0'
#client_id: 0012eb00f6d0

curl -iX POST \
  'http://localhost:4041/iot/devices' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
 "devices": [
   {
     "device_id":   "0012eb00f6d0",
     "entity_name": "0012eb00f6d0",
     "entity_type": "Raspberry",
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
###########################################################################
#### Dummy test measurements (for testing purposes): ######################
###########################################################################
###########################################################################

echo "#### Mosquitto dummy temperatura sensortag ####"
mosquitto_pub -h localhost -t /99/00124b05257a -m '033030|23.123'

sleep 1;

echo "#### Mosquitto dummy umidade ####"
mosquitto_pub -h localhost -t /99/00124b05257a -m '033040|80.789'

sleep 1;

echo "#### Mosquitto dummy RedLed ####"
mosquitto_pub -h localhost -t /99/00124b05257a -m '033110|1'

sleep 1;

echo "#### Mosquitto dummy GreenLed ####"
mosquitto_pub -h localhost -t /99/00124b05257a -m '033111|1'




echo "#### Mosquitto dummy temperatura remote ####"
mosquitto_pub -h localhost -t /99/00124b4a527d -m '033030|23.123'

sleep 1;

echo "#### Mosquitto dummy temperatura native ####"
mosquitto_pub -h localhost -t /99/010203060708 -m '033030|23.123'

sleep 1;

echo "#### Mosquitto dummy temperatura launchpad1 ####"
mosquitto_pub -h localhost -t /99/00124b82ad03 -m '033030|23.123'

sleep 1;

echo "#### Mosquitto dummy temperatura launchpad2 ####"
mosquitto_pub -h localhost -t /99/00124b82b206 -m '033030|23.123'

sleep 1;


#########################################################################################
#################################                        ################################
###################################                    ##################################
#################################### FIWARE - Cygnus ####################################
###################################                    ##################################
#################################                        ################################
#########################################################################################

##################
##### Subscription

echo "###### Cygnus - Subscription - VER ISSO!"

curl -iX POST \
  'http://localhost:1026/v2/subscriptions' \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
  "description": "Notify Cygnus of all context changes",
  "subject": {
    "entities": [
      {
        "idPattern": ".*"
      }
    ]
  },
  "notification": {
    "http": {
      "url": "http://cygnus:5050/notify"
    },
    "metadata": ["dateCreated", "dateModified"],
    "attrsFormat": "legacy"
  }
}'


sleep 1;


##################
##### Subscribe check

echo "############################################################"
echo "############################################################"
echo "########################## CHECKS ##########################"
echo "############################################################"
echo "############################################################"

sleep 1;

echo "############################################################"
echo "################### Cygnus Subscription ####################"
echo "############################################################"

curl -X GET \
  'http://localhost:1026/v2/subscriptions' \
  -H 'fiware-service: lwpubsub_schoolA' \
  -H 'fiware-servicepath: /fd00' | python -m json.tool



sleep 1;


echo "############################################################"
echo "############                                    ############"
echo "###########                                      ###########"
echo "##########          RESUMED DEVICES INFO          ##########"
echo "###########                                      ###########"
echo "############                                    ############"
echo "############################################################"

###################
####### Checks

#Native
# curl -X GET \
#   'http://localhost:1026/v2/entities/010203060708?type=LWPUBSUB&options=keyValues' \
#   -H 'fiware-service: school' \
#   -H 'fiware-servicepath: /fd00' | python -m json.tool

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


echo "############################################################"
echo "############                                    ############"
echo "###########                                      ###########"
echo "##########              SEND COMMAND!             ##########"
echo "###########                                      ###########"
echo "############                                    ############"
echo "############################################################"




curl -iX POST \
  http://localhost:4041/v2/op/update \
  -H 'Content-Type: application/json' \
  -H 'fiware-service: school' \
  -H 'fiware-servicepath: /fd00' \
  -d '{
    "actionType": "update",
    "entities": [
        {
            "type": "Native",
            "id": "010203060708",
            "33110" : {
                "type": "command",
                "value": "0"
            }
        }
    ]
}'

sleep 1;

echo -e "RED LED:"
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




echo -e "GREEN LED:"
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






echo -e "Alarm on:"
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
            "33380" : {
                "type": "command",
                "value": "1"
            }
        }
    ]
}'

sleep 4;

echo -e "Alarm off:"
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
            "33380" : {
                "type": "command",
                "value": "0"
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



# echo ""
# echo "Grafana info:"
# echo "http://localhost:3003/login"
# echo "user: admin"
# echo "senha: admin"
# echo ""
# echo "Add datasource: MySQL"
# echo "host: mysql-db:3306"
# echo "Database: lwpubsub_schoola"
# echo "User: root"
# echo "Pass: 123"
# echo ""
# echo ""
# echo "###### Example Grafana queries:"
# echo ""
# echo "###Temp"
# echo "SELECT UNIX_TIMESTAMP(recvtime) as time_sec, CAST((attrvalue) AS DECIMAL(7,3)) as value, 'temperatura' as metric FROM \`fd00_010203060708_LWPUBSUB\` WHERE attrname ='33030' ORDER BY time_sec;"
# echo "###RedLed"
# echo "SELECT UNIX_TIMESTAMP(recvtime) as time_sec, CAST((attrvalue) AS UNSIGNED) as value, 'RedLED' as metric FROM \`fd00_010203060708_LWPUBSUB\` WHERE attrname ='33030' ORDER BY time_sec;"
# echo ""





#################################################################################################
# FIWARE TUTORIAL:
# curl -iX POST \
#   'http://localhost:4041/iot/services' \
#   -H 'Content-Type: application/json' \
#   -H 'fiware-service: openiot' \
#   -H 'fiware-servicepath: /' \
#   -d '{
#  "services": [
#    {
#      "apikey":      "4jggokgpepnvsb2uv4s40d59ov",
#      "cbroker":     "http://orion:1026",
#      "entity_type": "Thing",
#      "resource":    ""
#    }
#  ]
# }'
#
#
#
#
#
#
#
#
#
#
# curl -iX POST \
#   'http://localhost:4041/iot/devices' \
#   -H 'Content-Type: application/json' \
#   -H 'fiware-service: openiot' \
#   -H 'fiware-servicepath: /' \
#   -d '{
#  "devices": [
#    {
#      "device_id":   "motion001",
#      "entity_name": "urn:ngsi-ld:Motion:001",
#      "entity_type": "Motion",
#      "protocol":    "PDI-IoTA-UltraLight",
#      "transport":   "MQTT",
#      "timezone":    "Europe/Berlin",
#      "attributes": [
#        { "object_id": "c", "name": "count", "type": "Integer" }
#      ],
#      "static_attributes": [
#        { "name":"refStore", "type": "Relationship", "value": "urn:ngsi-ld:Store:001"}
#      ]
#    }
#  ]
# }
# '
#
#
#
#
#
# mosquitto_pub -m "c|1" -t "/4jggokgpepnvsb2uv4s40d59ov/motion001/attrs"
#
#
#
#
#
#
#
#
#
#
#
#
# curl -X GET \
#   'http://localhost:1026/v2/entities/urn:ngsi-ld:Motion:001?type=Motion' \
#   -H 'fiware-service: openiot' \
#   -H 'fiware-servicepath: /' | python -m json.tool
#
#
#
#
#
#
#
#   curl -iX POST \
#   'http://localhost:4041/iot/devices' \
#   -H 'Content-Type: application/json' \
#   -H 'fiware-service: openiot' \
#   -H 'fiware-servicepath: /' \
#   -d '{
#   "devices": [
#     {
#       "device_id": "bell001",
#       "entity_name": "urn:ngsi-ld:Bell:001",
#       "entity_type": "Bell",
#       "protocol": "PDI-IoTA-UltraLight",
#       "transport": "MQTT",
#       "commands": [
#         { "name": "ring", "type": "command" }
#        ],
#        "static_attributes": [
#          {"name":"refStore", "type": "Relationship","value": "urn:ngsi-ld:Store:001"}
#       ]
#     }
#   ]
# }
# '
#
#
#
#
#
#
#
#
#
# curl -iX POST \
#   http://localhost:4041/v2/op/update \
#   -H 'Content-Type: application/json' \
#   -H 'fiware-service: openiot' \
#   -H 'fiware-servicepath: /' \
#   -d '{
#     "actionType": "update",
#     "entities": [
#         {
#             "type": "Bell",
#             "id": "urn:ngsi-ld:Bell:001",
#             "ring" : {
#                 "type": "command",
#                 "value": ""
#             }
#         }
#     ]
# }'
