INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= feasibility_tests.c feasibility_tests_updated.c tests.c cheddar_generate_test.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all: feasibility_tests feasibility_tests_updated cheddar_generate_test

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

cheddar_generate_test.o: cheddar_generate_test.c 
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

cheddar_generate_test:  cheddar_generate_test.o tests.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

cheddar: cheddar_generate_test
#https://stackoverflow.com/questions/47358120/how-to-pass-for-loop-variable-to-shell-function-in-makefile
	for i in `seq 0 9` ; do \
        for j in RM EDF LLF ; do \
                echo $$i $$j; \
                ./cheddar_generate_test $$j $$i > test_$${i}_$${j}.xml; \
            done \
    done

clean:
	-rm -f $(OBJS)
	-rm -f feasibility_tests feasibility_tests_updated cheddar_generate_test
	-rm -f *.xml 

