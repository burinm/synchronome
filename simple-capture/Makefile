INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g -Wall $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt

HFILES= 
CFILES= capture.c setup.c v4l2_capabilities.c buffer.c dumptools.c transformation.c sharpen.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	capture

clean: clean-frames
	-rm -f *.o *.d
	-rm -f capture sharpen test_starter

clean-frames:
	-rm -f *.yuv *.ppm *.pgm
	-rm -f timestamps.txt

capture: ${OBJS}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

#test_starter: capture_starter.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

#sharpen: __sharpen.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
