CFLAGS=-std=c++17 -O3 -Wall -lpthread
OBJS_TEST=switch.o fdb-entry.o dist-server.o test/dist-server-test.o
OBJS_TCP=switch.o fdb-entry.o dist-server.o tcp-server/tcp-dist-server.o tcp-server/tcp-server.o
OBJS_TCPTEST=tcp-test/dist-test_tcp.o
OBJS_TCPTAP=tcp-tap-client/tcp-tap-client.o
TARGETS=dist-server_tcp dist-server_test dist-server_tcp_test dist-client_tcp_tap
CC=g++

.PHONY: all
all: $(TARGETS)

dist-server_test: $(OBJS_TEST)
	$(CC) $(CFLAGS) -o dist-server_test $(OBJS_TEST)

dist-server_tcp: $(OBJS_TCP)
	$(CC) $(CFLAGS) -o dist-server_tcp $(OBJS_TCP)

dist-server_tcp_test: $(OBJS_TCPTEST)
	$(CC) $(CFLAGS) -o dist-server_tcp_test $(OBJS_TCPTEST)

dist-client_tcp_tap: $(OBJS_TCPTAP)
	$(CC) $(CFLAGS) -o dist-client_tcp_tap $(OBJS_TCPTAP)

%.o: %.cc
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS) *.o */*.o
