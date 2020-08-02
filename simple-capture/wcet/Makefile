all: flash_wcet motion_test 


flash_wcet: flash_wcet.o ../buffer.o ../dumptools.o ../timetools.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

flash_wcet.o: flash_wcet.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@


motion_test: motion_test.o ../buffer.o ../motion.o ../resources.o ../dumptools.o ../transformation.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ -lrt -lpthread

motion_test.o: motion_test.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

fake_capture.o: fake_capture.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

sequencer.o: sequencer.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

processing.o: processing.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<


clean:
	-rm -f *.o flash_wcet motion_test
	-rm -f *.ppm
	-rm -f ./errors/*[.yuv,.ppm,.pgm]
	-rm -f buffers.list
