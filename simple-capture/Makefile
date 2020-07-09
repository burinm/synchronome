INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g -Wall $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt

HFILES= 
CFILES= capture.c setup.c v4l2_capabilities.c buffer.c dumptools.c transformation.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	capture sharpen

clean:
	-rm -f *.o *.d
	-rm -f capture sharpen
	-rm -f *.yuv *.ppm

clean-frames:
	-rm -f *.yuv *.ppm

capture: ${OBJS}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

#test_starter: capture_starter.o
#	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

sharpen: sharpen.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
