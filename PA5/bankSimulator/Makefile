CC = gcc

CFLAGS = -Wall -g -lpthread

COMPILE = $(CC) $(CFLAGS)



all: client server



client: client.c client.h

	$(COMPILE) -o client client.c



server: server.c bank.h 

	$(COMPILE) -o server server.c



clean:

	rm -rf *.o *.dSYM client server
