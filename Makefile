OBJS = myshell.o
CC = gcc -std=gnu99
CFLAGS = -g -Wall

myshell: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -rf myshell *.dSYM
