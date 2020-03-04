CC=/usr/bin/g++
SOURCES=myredis.c mysock.c conn_server.c cJSON.c
TAR1=conn_server
CFLAGS= -O2 -Wall -std=c++11
LDS=-lhiredis -ljson
TAR2=test_client
all:$(TAR1) $(TAR2)
$(TAR1):$(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDS)
$(TAR2):test_client.c myredis.c myredis.h
	$(CC)  test_client.c myredis.c $(CFLAGS) $(LDS) -o $@

./PYHONY:install
install:$(TAR1)
	cp $(TAR1) /usr/bin/ -rf
	cp conn_redis.conf /etc/xrdp -rf
./PHONY:clean
clean:
	rm -rf *.o $(TAR1) test_client
