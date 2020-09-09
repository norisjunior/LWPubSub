#!/bin/bash
echo "Compiling - Native target"
make distclean
make TARGET=native MAKE_KEYSIZE=128 MAKE_CRYPTOMODE=CBC MAKE_WITH_ENERGY=0 MAKE_TIMEINTERVAL=MEDIUM
