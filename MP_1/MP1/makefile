all: crsd crc

interface.o: interface.h 
common.o: common.h

crc: crc.cpp  common.o
	g++ -g -w -std=c++11 -Wall -o crc crc.cpp  common.o

crsd: crsd.cpp common.o
	g++ -g -w -std=c++11 -Wall -o crsd crsd.cpp common.o -lpthread -lrt

clean:
	rm -rf *.o crsd crc
