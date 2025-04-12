all: myELF

myELF: myELF.o 
	gcc -m32 -g -Wall -o myELF myELF.o

myELF.o: myELF.c
	gcc -g -Wall -m32 -c -o myELF.o myELF.c 

.PHONY: clean 

# Clean the build directory 
clean:
	rm -f myELF.o myELF
