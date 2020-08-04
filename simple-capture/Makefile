INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
#CFLAGS= -pg -O0 -g -Wall $(INCLUDE_DIRS) $(CDEFS)
CFLAGS= -O0 -g -Wall $(INCLUDE_DIRS) $(CDEFS)

OBJS= camera.o v4l2_capabilities.o buffer.o camera_buffer.o camera_init.o resources.o motion.o dumptools.o transformation.o sharpen.o timetools.o memlog.o

all:	capture sequencer

clean: clean-frames
	-rm -f *.o *.d
	-rm -f sequencer capture sharpen test_starter
	sudo rm -f /dev/mqueue/frame_recieve_q  /dev/mqueue/writeout_q /dev/mqueue/processing_q
	rm -f sequencer.pid

clean-frames:
	-rm -f ./frames/*[.yuv,.ppm,.pgm]
	-rm -f ./errors/*[.yuv,.ppm,.pgm]
	-rm -f timestamps.txt
	-rm -f *.log

run: clean-frames
	sudo ./sequencer

sequencer: ${OBJS} sequencer.o frame.o processing.o writeout.o realtime.o memlog.o queue.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ -lrt -lpthread

capture: ${OBJS} frame_standalone.o capture_standalone.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ -lrt

capture_standalone.o: capture.c
	$(CC) -DCAPTURE_STANDALONE $(LDFLAGS) $(CFLAGS) -c $^ -o $@

frame_standalone.o: frame.c
	$(CC) -DCAPTURE_STANDALONE $(LDFLAGS) $(CFLAGS) -c $^ -o $@

#test_starter: capture_starter.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

#sharpen: __sharpen.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
