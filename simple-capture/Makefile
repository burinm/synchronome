INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g -Wall $(INCLUDE_DIRS) $(CDEFS)

OBJS= setup.o v4l2_capabilities.o buffer.o dumptools.o transformation.o sharpen.o timetools.o memlog.o 

all:	capture sequencer

clean: clean-frames
	-rm -f *.o *.d
	-rm -f sequencer capture sharpen test_starter

clean-frames:
	-rm -f *.yuv *.ppm *.pgm
	-rm -f timestamps.txt

sequencer: ${OBJS} sequencer.o capture.o processing.o writeout.o realtime.o memlog.o queue.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ -lrt -lpthread

capture: ${OBJS} capture_standalone.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ -lrt

capture_standalone.o: capture.c
	$(CC) -DCAPTURE_STANDALONE $(LDFLAGS) $(CFLAGS) -c $^ -o $@

#test_starter: capture_starter.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

#sharpen: __sharpen.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
