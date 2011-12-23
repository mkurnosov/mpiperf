#!/bin/sh

CURDIR=`pwd`
cd ..
make clean
make
cp -f src/mpiperf test/
cd $CURDIR
