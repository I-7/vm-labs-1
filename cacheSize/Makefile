all: main
	./main

../cacheLineSize/mainQuiet: ../cacheLineSize/main.cpp
	g++ -D QUIET -o ../cacheLineSize/mainQuiet ../cacheLineSize/main.cpp

main: ../cacheLineSize/mainQuiet main.cpp
	g++ -D CACHE_LINE_SIZE=$(shell ../cacheLineSize/mainQuiet) -o main main.cpp
