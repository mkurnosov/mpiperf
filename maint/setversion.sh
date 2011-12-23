#!/bin/sh

#
# Usage: setversion.sh MAJOR MINOR PATCH
# Example: setversion.sh 0 0 1
#
# This script setups version number in source code.
#


if [ $# -ne "3" ]; then
    echo "Usage: setversion.sh <MAJOR> <MINOR> <PATCH>"
    echo "Example: setversion.sh 0 0 1"
fi

echo "#define MPIPERF_VERSION_MAJOR $1" > ../src/version.h
echo "#define MPIPERF_VERSION_MINOR $2" >> ../src/version.h
echo "#define MPIPERF_VERSION_PATCH $3" >> ../src/version.h

echo "$1.$2.$3" > ../VERSION
