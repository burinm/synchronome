#!/bin/bash

PIDFILE="sequencer.pid"
if [ -e $PIDFILE ]
    then
        PID=`cat sequencer.pid`
        sudo kill -SIGABRT ${PID}
    else
        echo "sequencer not running"
fi


