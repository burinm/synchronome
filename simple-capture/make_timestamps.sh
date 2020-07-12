#!/bin/bash

SUFFIX=$1

if [ -z "$1" ]
then
    echo "usage make_timestamps <suffix>"
    exit
fi 

#Write blank file
echo -n > timestamps.txt

for i in `seq 0 999999`
do
    # Iterate through frame.000000 -> frame.nnnnnn in order 
    j=$(printf "frame.%06u.${SUFFIX}" "$i")
    sed -n 3p $j >> timestamps.txt 

    # As soon as we can't find a file, exit
    if [ $? -ne 0 ]
    then
        echo $j "done"
        exit
    fi
done

