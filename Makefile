CC=gcc
CFLAGS=-std=c99 -pedantic -g # -Wall
NAME=ifj2023
REMOVE = rm -f

run:
	$(CC) $(CFLAGS) *.c -o $(NAME)

clean:
	$(REMOVE) *.o $(NAME)