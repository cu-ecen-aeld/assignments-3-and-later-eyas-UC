#!/bin/sh


if [ $# -gt 2 ]; then
    echo "Too many argments"
    exit 1
elif [ $# -lt 2 ]; then
    echo "too few arguemnts"
    exit 1
else
    writefile=$1
    writestr=$2
    if [ -e FILE ]; then
        echo "writing  $writestr in $writefile"
        echo "$writestr" > $writefile
    else
        mkdir -p $writefile
        rm -rf $writefile
        echo "$writestr" > $writefile
    fi

fi