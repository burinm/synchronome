#!/bin/bash
# Take down unruly invocation of synchonome

PIDFILE="sequencer.pid"
if [ -e $PIDFILE ]
    then
        PID=`cat sequencer.pid`
        sudo kill -SIGABRT ${PID}
    else
        echo "sequencer not running"
fi


