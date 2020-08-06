#!/bin/bash

v4l2-ctl -c exposure_auto=1
v4l2-ctl -c exposure_absolute=60

v4l2-ctl -C exposure_auto
v4l2-ctl -C exposure_absolute

#v4l2-ctl -c focus_auto=0
#v4l2-ctl -c focus_absolute=115

#v4l2-ctl -C focus_auto
#v4l2-ctl -C focus_absolute




