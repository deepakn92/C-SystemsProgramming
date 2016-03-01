CC = gcc
CFLAGS  = -Wall -std=c99
TARGET = main

all: $(TARGET)

$(TARGET): libsl.a main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c libsl.a

libsl.a:  sorted-list.o
	ar rcu libsl.a sorted-list.o
	ranlib libsl.a

sorted-list.o:  sorted-list.c sorted-list.h 
	$(CC) $(CFLAGS) -c sorted-list.c
	
clean:
	rm $(TARGET) *.o
sl:
	./main