#!/bin/bash

echo "#############################################################"
echo "###      CWIoT - compiling - Sensortag CC2650 target      ###"
echo "#############################################################"

if [[ $# -eq 0 ]] ; then
    echo 'usage: fixed AES-CBC-256 encryption mode, provide:'
    echo '- poll frequency: VERYHIGH, HIGH, MEDIUM, LOW, VERYLOW (case sensitive)'
    echo '- TSCH schedule: MINIMAL or ORCHESTRA'
    echo 'example: Very high pool frequency: make_lwpubsub_sensortag-poll.sh VERYHIGH ORCHESTRA'
    echo ''
    exit 0
fi

if   [ "$1" != 'VERYHIGH' ] && [ "$1" != 'HIGH' ] && [ "$1" != 'MEDIUM' ] && [ "$1" != 'LOW' ] && [ "$1" != 'VERYLOW' ] ; then
  echo 'error: only VERYHIGH, HIGH, MEDIUM, LOW, and VERYLOW allowed modes'
  exit 0
fi

if   [ "$2" != 'MINIMAL' ] && [ "$2" != 'ORCHESTRA' ] ; then
  echo 'error: only MINIMAL or ORCHESTRA allowed TSCH schedules'
  exit 0
fi

if   [ "$2" == 'ORCHESTRA' ] ; then
  is_orchestra=1
fi



dir_binaries=~/Google\ Drive/Doutorado/#Artigos/LWPubSub\ for\ IoT/Experiments/binaries

make distclean

make TARGET=cc26x0-cc13x0 BOARD=sensortag/cc2650 MAKE_KEYSIZE=256 MAKE_CRYPTOMODE=CBC MAKE_WITH_ENERGY=1 MAKE_POLLFREQUENCY="$1" MAKE_WITHSCHEDORCHESTRA=$is_orchestra

cp build/cc26x0-cc13x0/sensortag/cc2650/lwsec-pubsub-system.hex "$dir_binaries/lwsec-pubsub-system-"$2"-AES-CBC-256-"$1".hex"
