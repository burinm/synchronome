PROGRAM=schedule

all: $(PROGRAM)
CFLAGS += -g

$(PROGRAM): schedule.o ../feasibility/tests.o
	$(CC) $(CFLAGS) $^ -o $@ -lm

schedule.o: schedule.c
	$(CC) $(CFLAGS) -c $? -o $@

tests.o: ../feasibility/tests.c
	$(CC) $(CFLAGS) -c $? -o $@ -lm

clean:
	rm -rf $(PROGRAM)
	rm -rf *.o
	rm -rf ../feasibility/tests.o
