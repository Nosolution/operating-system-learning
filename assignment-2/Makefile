CXX = g++
CC = gcc
ASM = nasm
objects = main.o dirnode.o direntry.o fat12header.o commons.o print.o my_print.o
# objects = main.o dirnode.o direntry.o fat12header.o commons.o 
target = main

$(target): $(objects)
	$(CXX)  -m32 -g $(objects) -o $(target) 


# %.o: %.cpp
# 	$(CXX)  -c $^ -g -Wall

%.o: %.cpp
	$(CXX) -m32 -c $^ -g -Wall

my_print.o: my_print.asm
	$(ASM) -g -f elf $^

print.o: print.c
	$(CC) -m32 -c $^ -g -Wall

.PHONY: clean
clean:
	rm -rf $(target) $(objects)
