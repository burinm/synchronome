#!/bin/bash
# Gracefully stop the sequencer in its tracks

PIDFILE="sequencer.pid"
if [ -e $PIDFILE ]
    then
        PID=`cat sequencer.pid`
        sudo kill -SIGRTMIN ${PID}
    else
        echo "sequencer not running"
fi


