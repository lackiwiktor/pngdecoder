CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
decoder:
	$(CC) $(CFLAGS) -o decoder decoder.c 
