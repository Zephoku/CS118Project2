CC=g++
CFLAGS=-c -Wall
LDFLAGS=

all: server client
	
server: 
	$(CC) $(LDFLAGS) server.cpp -o server

client:
	$(CC) $(LDFLAGS) client.cpp -o client

clean: 
	rm server client
