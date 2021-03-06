#!/bin/bash

echo "#############################################################"
echo "###                     Toggle buzzer                     ###"
echo "#############################################################"

if [[ $# -eq 0 ]] ; then
    echo 'usage: toggle-buzzer <device_name/type> <deviceID>'
    exit 0
fi

echo $1

if   [ -z "$1" ] ; then
  echo 'error: provide device_name/type'
  exit 0
fi

if   [ -z "$2" ] ; then
  echo 'error: provide deviceID'
  exit 0
fi


devicename=$1

deviceID=$2



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
            "type": "'"$devicename"'",
            "id": "'"$deviceID"'",
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
            "type": "'"$devicename"'",
            "id": "'"$deviceID"'",
            "33380" : {
                "type": "command",
                "value": "0"
            }
        }
    ]
}'
