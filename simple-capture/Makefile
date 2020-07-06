INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g -Wall $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt

HFILES= 
CFILES= capture.c setup.c v4l2_capabilities.c dumptools.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	capture

clean:
	-rm -f *.o *.d
	-rm -f capture

clean-all:
	-rm -f *.o *.d
	-rm -f capture
	-rm -f *.yuv 

capture: ${OBJS}
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
