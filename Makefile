OBJS	= main.o scanner.o parser.o
SOURCE	= main.cpp scanner.cpp parser.cpp
HEADER	= scanner.h parser.h
OUT	    = compfs
CC	    = g++
FLAGS	= -g -c -Wall

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) -std=c++11

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp -std=c++11

scanner.o: scanner.cpp
	$(CC) $(FLAGS) scanner.cpp -std=c++11

parser.o: parser.cpp
	$(CC) $(FLAGS) parser.cpp -std=c++11

clean:
	rm -f $(OBJS) $(OUT)