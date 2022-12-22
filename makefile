CC = gcc
CFLAGS = -Wall -g
LIB = -lm -L ./ -lbloom
MAKE = make

shelfish.out: shelfish.o element.o avl.o sstable.o
	$(CC) $(CFLAGS) -o $* $^ $(LIB)

clean: 
	rm -rf *.o *.out shelfish
	rm -rf ./.shelves

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean all