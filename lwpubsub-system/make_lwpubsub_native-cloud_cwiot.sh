#!/bin/bash

echo "#############################################################"
echo "###      CWIoT - compiling - Sensortag CC2650 target      ###"
echo "#############################################################"

if [[ $# -eq 0 ]] ; then
    echo 'usage: provide:'
    echo '- crypto algorithm .: CBC, or CTR (case sensitive)'
    echo '- crypto key .......: 128, 192, or 256)'
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


make distclean

make TARGET=native MAKE_KEYSIZE="$2" MAKE_CRYPTOMODE="$1"
