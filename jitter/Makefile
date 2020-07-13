PROGRAM=jitter

all: $(PROGRAM)
CFLAGS += -lpthread -lrt -g

$(PROGRAM): sequencer.o realtime.o timetools.o memlog.o
	$(CC) $(CFLAGS) $^ -o $@

sequencer.o: sequencer.c
	$(CC) $(CFLAGS) $? -c -o $@

realtime.o: realtime.c
	$(CC) $(CFLAGS) $? -c -o $@

timetools.o: timetools.c
	$(CC) $(CFLAGS) $? -c -o $@

memlog.o: memlog.c
	$(CC) $(CFLAGS) $? -c -o $@

clean:
	rm -rf $(PROGRAM)
	rm -rf *.o 
