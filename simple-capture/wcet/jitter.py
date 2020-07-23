#!/usr/bin/env python3

# timediff.py - burin (c) 2020
#   Output events and relative time deltas from timestamp file   

import sys
import re

# https://stackoverflow.com/questions/3762881/how-do-i-check-if-stdin-has-some-data
if sys.stdin.isatty() or len(sys.argv) < 2:
    print("usage: jitter <base> [thresh max] [thresh min] (stdin)");
    sys.exit(0)

base = float(sys.argv[1])
thresh_max = None
thresh_min = None

if len(sys.argv) > 2:
    thresh_max = float(sys.argv[2])

if len(sys.argv) > 3:
    thresh_min = float(sys.argv[3])

    

try:
    data_file = sys.stdin
except:
    print("Couldn't open <stdin>")
    sys.exit(0)


max_jitter = 0
min_jitter = 500 # Should be whatever python "INT_MAX" is 

for l in data_file:
    l = l.strip();
    if l[:1] == '#':
        continue 
    else:
        num_string = l.strip()
        if num_string == '':
            continue

        value = float(l)

        if thresh_max is not None:
            if value > thresh_max:
                print("ignoring max:", value)
                continue

        if thresh_min is not None:
            if value < thresh_min:
                print("ignoring min:", value)
                continue

        jitter = value - base
        if jitter > max_jitter:
            max_jitter = jitter

        if jitter < min_jitter:
            min_jitter = jitter

print("max jitter:{:9.3f}".format(max_jitter))
print("min jitter:{:9.3f}".format(min_jitter))
