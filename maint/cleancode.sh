#! /bin/bash

#
# Usage: cleancode.sh <filename>
# This tool formats source code.

if [ -z "$1" ]; then
    echo "Usage: $0 <filename>"
    exit
fi

indent --k-and-r-style -bad --line-length80 --comment-line-length80 \
       --else-endif-column1 --start-left-side-of-comments \
       --break-after-boolean-operator --dont-cuddle-else \
       --dont-format-comments --comment-indentation1 --indent-level4 \
       --no-tabs $1
rm -f $1~

