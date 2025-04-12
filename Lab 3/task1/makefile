#format is target-name: target dependencies 

#{-tab-}actions 

 # All Targets 

.PHONY : clean

all: main

main :  start.o util.o
	ld -m elf_i386 start.o util.o -o main

#main.o : main.s
 	#nasm -f elf32 main.s -o main.o

util.o : util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o : start.s
	nasm -f elf32 start.s -o start.o		
clean:
	rm -f *.o main