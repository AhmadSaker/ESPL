#format is target-name: target dependencies 

#{-tab-}actions 

 # All Targets 


.PHONY : clean

all: main

main :  start.o util.o main.o
	ld -m elf_i386 start.o main.o util.o -o task2

main.o : main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o 

util.o : util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o : start.s
	nasm -f elf32 start.s -o start.o 

clean:
	rm -f *.o main task2
