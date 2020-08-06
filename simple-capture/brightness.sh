#!/bin/bash

v4l2-ctl -c exposure_auto=1
v4l2-ctl -c exposure_absolute=60

v4l2-ctl -C exposure_auto
v4l2-ctl -C exposure_absolute




