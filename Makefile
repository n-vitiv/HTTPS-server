CC = gcc

INCLUDE = src/http.c src/network.c src/logger.c src/utils.c src/configparser.c src/ssl.c

LIBRARY =  -lssl -lcrypto -lpthread 

.PHONY: all clean

all:
	$(CC) main.c $(INCLUDE) -o server $(LIBRARY)

clean:
	rm server
