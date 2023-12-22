CFLAGS = -O
CC = g++

main.exe: main.o tsp.o
	$(CC) $(CFLAGS) -o main.exe main.o tsp.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

tsp.o: tsp.cpp
	$(CC) $(CFLAGS) -c tsp.cpp

clean:
	rm -f core *.o main.exe
