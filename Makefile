CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES= Window.cpp Header.cpp Data.cpp

all: server client
	
server: 
	$(CC) $(LDFLAGS) $(SOURCES) server.cpp -o server

client:
	$(CC) $(LDFLAGS) $(SOURCES) client.cpp -o client

clean: 
	rm server client
