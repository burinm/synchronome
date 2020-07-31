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

# Don't order, just glob all the image files
for j in `ls -1 ${DIR}/*.${SUFFIX}`
do
    TIMESTAMP_LINE=$(sed -n 3p $j)

    # As soon as we can't find a file, exit
    if [ $? -ne 0 ]
    then
        echo $j "done"
        exit
    fi

    echo $TIMESTAMP_LINE | sed 's/#//'
done

