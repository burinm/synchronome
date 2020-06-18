INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= feasibility_tests.c feasibility_tests_updated.c tests.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	feasibility_tests feasibility_tests_updated

feasibility_tests.o: feasibility_tests.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $? -o $@

feasibility_tests: feasibility_tests.o
	$(CC) $(LDFLAGS) $(CFLAGS) $? -o $@ -lm

tests.o: tests.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $? -o $@

feasibility_tests_updated.o: feasibility_tests_updated.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $? -o $@

feasibility_tests_updated: feasibility_tests_updated.o tests.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ -lm

clean:
	-rm -f $(OBJS)
	-rm -f feasibility_tests feasibility_tests_updated

