CC = gcc
CFLAGS = -Wall -g -fopenmp
MAKE = make

shelfish.out: shelfish.o element.o avl.o
	$(CC) $(CFLAGS) -o $@ $^

clean: 
	rm -rf *.o *.out shelfish
	rm -rf ./.shelf

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean all