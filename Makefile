CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES= Window.cpp Header.cpp 

all: server client
	
server: 
	$(CC) $(LDFLAGS) $(SOURCES) server.cpp -o server

client:
	mkdir clientfiles && $(CC) $(LDFLAGS) $(SOURCES) client.cpp -o clientfiles/client

test:
	$(CC) $(LDFLAGS) $(SOURCES) test.cpp -o test

clean: 
	rm server; rm -r clientfiles

