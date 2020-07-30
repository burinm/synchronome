#!/bin/bash

DIR=$1
SUFFIX=$2

if [ -z "$2" ]
then
    echo "usage: make_timestamps <dir> <suffix>"
    exit
fi 

#Write blank file
echo -n > timestamps.txt

for i in `seq 0 999999`
do
    # Iterate through frame.000000 -> frame.nnnnnn in order 
    j=$(printf "${DIR}/frame.%06u.${SUFFIX}" "$i")
    TIMESTAMP_LINE=$(sed -n 3p $j)

    # As soon as we can't find a file, exit
    if [ $? -ne 0 ]
    then
        echo $j "done"
        exit
    fi

    echo $TIMESTAMP_LINE | sed 's/#//' >> timestamps.txt
done

