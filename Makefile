CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
heap:
	$(CC) $(CFLAGS) -o decoder decoder.c 
