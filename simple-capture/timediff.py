#!/usr/bin/env python3

# timediff.py - burin (c) 2020
#   Output events and relative time deltas from timestamp file   

import sys

if len(sys.argv) != 2:
    print("usage: timediff <timing_file.txt>"); 
    sys.exit(0)


data_file = None

try:
    data_file = open(sys.argv[1], "r")
except:
    print("Couldn't open", sys.argv[1]); 
    sys.exit(0)
    
timestamp = None
first_timestamp = None
last_timestamp = None
last_service = None

first_entry = True

count = 0
total = 0

for l in data_file:
    l = l.strip();
    if l[:1] == '#':
        timestamp = l.split('#')
        values = timestamp[1].split()

        # nnnnnnnnnn sec mmmmmmmmm nsec
        if len(values) == 4:
            (seconds, dummy1, nanoseconds, dummy2) = values

            timestamp = (int(seconds) * 1000000000) + int(nanoseconds)

            # Timestamps are in ns, convert to milliseconds 
            timestamp = timestamp / 1000000

            if last_timestamp != None:
                diff = timestamp - last_timestamp;
                count = count + 1
                total += diff
                print("{:06.3f}".format(diff), end = '\n')

            last_timestamp = timestamp

        else:
            pass
    else:
        pass

average = total / count
print("average: {:06.3f}".format(average), end = '\n')
