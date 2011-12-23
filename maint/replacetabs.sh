#!/bin/sh

#
# Usage: replace-tabs.sh <dir>
# This tool replaces all tabs in *.c and *.h files to 4 spaces.
#

if [ "$#" -lt "1" ]; then
    echo "Usage: $0 <dir>"
    exit
fi

find $1 -name *.[c,h] -type f -print | while read fname; do
    echo $fname
    expand -t4 $fname > $fname.notabs
    mv -f $fname.notabs $fname
done


