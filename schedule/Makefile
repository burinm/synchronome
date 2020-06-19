PROGRAM=schedule

all: $(PROGRAM)

$(PROGRAM): schedule.c ../feasibility/tests.c
	$(CC) $(CFLAGS) -g $? -o $@ -lm

clean:
	rm -rf $(PROGRAM)
	rm -rf *.o
