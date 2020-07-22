all: flash_wcet buffer_wcet


flash_wcet: flash_wcet.o ../buffer.o ../dumptools.o ../timetools.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

flash_wcet.o: flash_wcet.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@


buffer_wcet: fake_capture.o sequencer.o processing.o ../buffer.o ../queue.o ../motion.o ../memlog.o ../timetools.o ../realtime.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ -lrt -lpthread

fake_capture.o: fake_capture.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

sequencer.o: sequencer.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

processing.o: processing.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<


clean:
	-rm -f *.o flash_wcet
	-rm -f *.ppm
