CFLAGS=-std=c++11 -O3 -Wall -lpthread
OBJS_TEST=distributor.o dist-server.o test/dist-server-test.o
OBJS_TCP=distributor.o dist-server.o tcp-server/tcp-dist-server.o tcp-server/tcp-server.o
CC=g++

.PHONY: all
all: dist-server_tcp dist-server_test

dist-server_test: $(OBJS_TEST)
	$(CC) $(CFLAGS) -o dist-server_test $(OBJS_TEST)

dist-server_tcp: $(OBJS_TCP)
	$(CC) $(CFLAGS) -o dist-server_tcp $(OBJS_TCP)

%.o: %.cc
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm dist-server_test dist-server_tcp  *.o */*.o
