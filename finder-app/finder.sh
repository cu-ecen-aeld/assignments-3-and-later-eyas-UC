#!/bin/sh
if [ $# -gt 2 ]; then
    echo "ERROR: too many parameters, should pass only 2. However  $# are passed"
    exit 1
elif [ $# -lt 2 ]; then
    echo "ERROR: missing parameters, should pass only 2. However Only $# is passed"
    exit 1
elif [ $# -eq 2 ]; then
    filesdir=$1
    searchstr=$2
    # echo "filesdir=$filesdir" 
    # echo "searchstr=$searchstr" 

    if [ "$filesdir" = "." ]; then
        filesdir=$PWD
        echo "updated path to $PWD"
    fi
    if [ -d $filesdir ]; then
        tmp_file=/tmp/finder_script_temp_file
        # echo "running the command grep -rc $searchstr $filesdir/* "
        # grep -rcEo $searchstr $filesdir/* | grep -Eo '[0-9]+$' > $tmp_file
        # grep -rcEo $searchstr $filesdir/* | grep -Eo '[0-9]+$' 
        # grep -rcEo $searchstr $filesdir/* |  awk 'BEGIN {FS=":"} {print $NF }' > $tmp_file
        grep -rc $searchstr $filesdir/* |  awk 'BEGIN {FS=":"} {print $NF }' > $tmp_file

        sum=0
        lines=0
        # echo "file is $tmp_file"
        # echo "------------------------------------------------"
        # cat  $tmp_file
        # echo "------------------------------------------------"
        while read -r num; do
        # echo "num = $num sum = $sum and lines = $lines"
        sum=$((sum + num))
        if [ $num -ne 0 ]; then
            lines=$((lines + 1))
        fi
        done < $tmp_file



        # number_of_matches=$(awk '{ sum += $1 } END { print sum }' $tmp_file)
        # echo "number of matches = $sum and files is $lines"
        echo "The number of files are ${lines} and the number of matching lines are ${sum}"
        exit 0
        # rm $tmp_file

    else
        echo "ERROR: invalid path "
    fi

fi
