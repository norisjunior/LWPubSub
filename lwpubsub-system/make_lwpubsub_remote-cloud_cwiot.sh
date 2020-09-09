#!/bin/bash

echo "#############################################################"
echo "###      CWIoT - compiling - Sensortag CC2650 target      ###"
echo "#############################################################"

if [[ $# -eq 0 ]] ; then
    echo 'usage: provide:'
    echo '- crypto algorithm .: CBC, or CTR (case sensitive)'
    echo '- crypto key .......: 128, 192, or 256)'
    echo '- TSCH schedule ....: MINIMAL or ORCHESTRA'
    echo ''
    echo 'Poll frequency fixed at 1 message each 30 seconds'
    echo ''
    echo 'examples:'
    echo '-> AES-CBC-128, ORCHESTRA schedule: make_lwpubsub_sensortag-cloud_cwiot.sh CBC 128 ORCHESTRA'
    echo '-> AES-CTR-192, MINIMAL schedule: make_lwpubsub_sensortag-cloud_cwiot.sh CTR 192 MINIMAL'
    echo ''
    exit 0
fi


if   [ "$1" != 'CBC' ] && [ "$1" != 'CTR' ] ; then
  echo 'error: only "CBC" of "CTR" allowed modes'
  exit 0
fi


if   [ "$2" != '128' ] && [ "$2" != '192' ] && [ "$2" != '256' ] ; then
  echo 'error: use only 128, 192, 256-bit keys'
  exit 0
fi

if   [ "$3" != 'MINIMAL' ] && [ "$3" != 'ORCHESTRA' ] ; then
  echo 'error: only MINIMAL or ORCHESTRA allowed TSCH schedules'
  exit 0
fi

if   [ "$3" == 'ORCHESTRA' ] ; then
  is_orchestra=1
fi



#dir_binaries=~/Google\ Drive/Doutorado/#Artigos/LWPubSub\ for\ IoT/Experiments/binaries
#dir_footprint=~/Google\ Drive/Doutorado/#Artigos/LWPubSub\ for\ IoT/Experiments/footprint
#file_flashprof=$dir_footprint"/lwsec-pubsub-system-$4-AES-$1-$2-$3-flashprof-sensortag.log"
#file_ramprof=$dir_footprint"/lwsec-pubsub-system-$4-AES-$1-$2-$3-ramprof-sensortag.log"

#make distclean

#make lwsec-pubsub-system.flashprof TARGET=cc26x0-cc13x0 BOARD=sensortag/cc2650 MAKE_KEYSIZE="$2" MAKE_CRYPTOMODE="$1" > "$file_flashprof"

#make lwsec-pubsub-system.ramprof TARGET=cc26x0-cc13x0 BOARD=sensortag/cc2650 MAKE_KEYSIZE="$2" MAKE_CRYPTOMODE="$1" > "$file_ramprof"

make distclean

make TARGET=zoul BOARD=remote-revb PORT=/dev/ttyUSB1 MAKE_KEYSIZE="$2" MAKE_CRYPTOMODE="$1" MAKE_WITH_ENERGY=1 MAKE_WITHSCHEDORCHESTRA=$is_orchestra lwsec-pubsub-system.upload

#cp build/cc26x0-cc13x0/sensortag/cc2650/lwsec-pubsub-system.hex "$dir_binaries/lwsec-pubsub-system-Cloud_CWIoT-AES-$1-$2-$3.hex"
