CC=gcc
grand: grand.c 
	$(CC) -Og grand.c -o grand -lX11 -lm -lasound -lpthread
all: grand 
