#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "usage: make_frames_list <dir>"
    exit 0
fi

DIR=$1

ls -1 ${DIR}/*.ppm | sort > frames.list
