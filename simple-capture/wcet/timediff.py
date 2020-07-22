#!/usr/bin/env python3

# timediff.py - burin (c) 2020
#   Output events and relative time deltas from timestamp file   

import sys
import re

if len(sys.argv) < 2:
    print("usage: timediff [file/-] [SERVICE 0..n]");
    sys.exit(0)

data_file = None
use_stdin = False

if sys.argv[1] == '-':
    use_stdin = True

try:
    if use_stdin == True:
        data_file = sys.stdin
    else:
        data_file = open(sys.argv[1], "r")
except:
    print("Couldn't open", sys.argv[1])
    sys.exit(0)

services = []
for s in sys.argv[2:]:
    services.append(s)


timestamp = None
first_timestamp = None
last_timestamp = None

first_entry = True

for l in data_file:
    l = l.strip();
    if l[:1] == '#':
        continue 
    else:
        values = l.split()
        if len(values) >= 2:

            # remove_brackets = re.compile(r"^\[\b(\w*)\]$")

            # All logs have at least service and timestamp
            # values[0] timestamp
            # values[1] log entry name, service
            timespec = values[0] # struct timespec from C
            service = values[1]
            #match = remove_brackets.search(values[1])
            #if not match:
            #    print("log corrupted? {}".format(values[1]))
            #    continue

            #service = match.group(1)
            #print("S:", service)

            match = True # If no services given, always process
            for s in services:
                match = False

                #print("is ", service, " == ", s)
                if service == s:
                    match = True
                    break

            if not match:
                continue

            #for d in values[2:]:
            #    print("data:", d)

            # Timespecs are in s.ns, convert to usec 
            (seconds, nanoseconds) = timespec.split(".")
            timestamp = int((int(seconds) * 1000000) + (int(nanoseconds) / 1000))


            #print("service", service, " us ", timestamp)


            print("{}.{} {:<25}".format(seconds, nanoseconds, service), end = ' ')
            if last_timestamp != None:
                # show diffs in ms
                diff = (timestamp - last_timestamp) / 1000
                print("{: 9.3f}".format(diff), end = '\n')
            else:
                print("")


            last_timestamp = timestamp

        else:
            pass

print("");
