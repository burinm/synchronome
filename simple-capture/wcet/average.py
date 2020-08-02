#!/usr/bin/env python3

# average.py - burin (c) 2020
#   average a column of numbers   

import sys
import re

# https://stackoverflow.com/questions/3762881/how-do-i-check-if-stdin-has-some-data
if sys.stdin.isatty() or len(sys.argv) < 1:
    print("usage: average [thresh max] [thresh min] (stdin)");
    sys.exit(0)

thresh_max = None
thresh_min = None

if len(sys.argv) > 1:
    thresh_max = float(sys.argv[1])

if len(sys.argv) > 2:
    thresh_min = float(sys.argv[2])
    

try:
    data_file = sys.stdin
except:
    print("Couldn't open <stdin>")
    sys.exit(0)

total_value = 0
total_count = 0

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

        total_value += value;
        total_count += 1;

print("Average:{:9.6f}".format(total_value/total_count))
