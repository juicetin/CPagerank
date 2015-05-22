CC = clang
CFLAGS = -O0 -g -W -Wall -pedantic -std=gnu11 -march=native
LDFLAGS = -lm -lpthread

all: pagerank

pagerank: pagerank.o

clean:
	-rm -f *.o
	-rm -f pagerank