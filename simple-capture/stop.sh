#!/bin/bash

PIDFILE="sequencer.pid"
if [ -e $PIDFILE ]
    then
        PID=`cat sequencer.pid`
        sudo kill -SIGUSR1 ${PID}
    else
        echo "sequencer not running"
fi


