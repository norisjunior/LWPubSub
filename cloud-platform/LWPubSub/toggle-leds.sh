#!/bin/bash

echo "#############################################################"
echo "###                      Toggle leds                      ###"
echo "#############################################################"

if [[ $# -eq 0 ]] ; then
    echo 'usage: toggle-leds <led> <on/off> <device_name/type> <deviceID>'
    exit 0
fi

echo $1

if   [ -z "$3" ] ; then
  echo 'error: provide device_name/type'
  exit 0
fi

if   [ -z "$4" ] ; then
  echo 'error: provide deviceID'
  exit 0
fi

if   [ "$1" != 'red' ] && [ "$1" != 'green' ] ; then
  echo 'error: only \"red\" of \"green\" allowed modes'
  exit 0
fi


if   [ "$2" != 'on' ] && [ "$2" != 'off' ] ; then
  echo 'error: only \"red\" of \"green\" allowed modes'
  exit 0
fi

#Default = turn on
action=1

devicename=$3

deviceID=$4

#text=$(tr -d ' ' <<< "$text")

redled () {
  if [ "$1" == 'on' ] ; then
  	action=1
  else
  	action=0
  fi
  echo "red" $action

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
              "33110" : {
                  "type": "command",
                  "value": "'$action'"
              }
          }
      ]
  }'

}



greenled () {
  if [ "$1" == 'on' ] ; then
  	action=1
  else
  	action=0
  fi
  echo "green" $action

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
              "33111" : {
                  "type": "command",
                  "value": "'$action'"
              }
          }
      ]
  }'

}


case "$1" in
    'red')   redled $2 ;;
    'green')  greenled $2 ;;
    *) echo 'error: only \"red\" or \"green\" and \"on\" of \"off\" allowed modes' ;;
esac
