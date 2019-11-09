CC = g++
objects = main.o dirnode.o direntry.o fat12header.o commons.o
target = main

$(target): $(objects)
	$(CC) $(objects) -o $(target) 

%.o: %.cpp
	$(CC) -c $^ -g -Wall -static-libgcc
# direntry.o: direntry.h direntry.cpp
# 	$(CC) -c $^ -o $@
# fat12header.o: fat12header.h fat12header.cpp
# 	$(CC) -c $^ -o $@
# dirnode.o: dirnode.h dirnode.cpp
# 	$(CC) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(target) $(objects)
