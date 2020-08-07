#!/bin/bash
# Used for error dump, get list of YUYV frames for motion_test 

ls -1 ../errors/*.yuv | sort > buffers.list
