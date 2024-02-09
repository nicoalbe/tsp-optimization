CFLAGS = -O
CC = g++

main.exe: main.o methods/tsp.o
	$(CC) $(CFLAGS) -o main.exe main.o methods/tsp.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

tsp.o: tsp.cpp
	$(CC) $(CFLAGS) -c methods/tsp.cpp

clean:
	rm -f core *.o main.exe
