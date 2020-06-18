INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= feasibility_tests.c feasibility_tests_updated.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	feasibility_tests feasibility_tests_updated

clean:
	-rm -f *.o *.d
	-rm -f feasibility_tests

feasibility_tests: feasibility_tests.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lm

feasibility_tests_updated: feasibility_tests_updated.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lm

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
