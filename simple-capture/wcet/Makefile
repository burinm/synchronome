all: flash_wcet    	


flash_wcet: flash_wcet.o ../buffer.o ../dumptools.o ../timetools.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

flash_wcet.o: flash_wcet.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<


clean:
	-rm -f *.o flash_wcet
	-rm -f *.ppm 
