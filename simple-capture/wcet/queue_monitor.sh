#!/bin/bash

FRAME_BUFFER_T_SZ=8
FRAME_BUF_COUNT=10

WO_BUFFER_T_SZ=8
WO_BUF_COUNT=300

clear

while [ 1 ]
    do
        # subfields! http://www.bolthole.com/awk5.html
        DEPTH=$(sudo cat /dev/mqueue/frame_recieve_q | awk  '{ newline=$1; split(newline,subfields,":"); print subfields[2] }')
        DEPTH=$((DEPTH / FRAME_BUFFER_T_SZ))

        # progress bar! https://stackoverflow.com/questions/23630501/how-to-progress-bar-in-bash
        for ((i=0 ; i < $DEPTH ; i++))
            do
                echo -n "*"
        # use same line https://stackoverflow.com/questions/47813665/progress-bar-eta-on-same-line-with-pv-command
        done | (pv -f -b -p -c -s $FRAME_BUF_COUNT) > /dev/null

    
        DEPTH=$(sudo cat /dev/mqueue/writeout_q | awk  '{ newline=$1; split(newline,subfields,":"); print subfields[2] }')
        DEPTH=$((DEPTH / WO_BUFFER_T_SZ))

        for ((i=0 ; i < $DEPTH ; i++))
            do
                echo -n "*"
            done | (pv -f -b -p -c -s $WO_BUF_COUNT) > /dev/null

        # https://www.tldp.org/HOWTO/Bash-Prompt-HOWTO/x361.html
        echo -ne "\033[2A"

done
