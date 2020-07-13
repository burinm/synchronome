PROGRAM=jitter

all: $(PROGRAM)
CFLAGS += -lpthread -lrt -g

$(PROGRAM): sequencer.o realtime.o
	$(CC) $(CFLAGS) $^ -o $@

sequencer.o: sequencer.c
	$(CC) $(CFLAGS) $? -c -o $@

realtime.o: realtime.c
	$(CC) $(CFLAGS) $? -c -o $@

clean:
	rm -rf $(PROGRAM)
	rm -rf *.o 
