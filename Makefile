CC=zig cc
CFLAGS= -ffreestanding -nostdinc -target x86_64-linux-gnu -O3 

build: syscall.o
	$(CC) syscall.o -nostdlib -static -o example

syscall.o: example.c syscall.h
	$(CC) $(CFLAGS) -c example.c -o syscall.o

