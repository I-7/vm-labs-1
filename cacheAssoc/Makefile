all: main
	./main

../cacheLineSize/mainQuiet: ../cacheLineSize/main.cpp
	g++ -D QUIET -o ../cacheLineSize/mainQuiet ../cacheLineSize/main.cpp

../cacheSize/mainQuiet: ../cacheLineSize/mainQuiet ../cacheSize/main.cpp
	g++ -D QUIET -D CACHE_LINE_SIZE=$(shell ../cacheLineSize/mainQuiet) -o ../cacheSize/mainQuiet ../cacheSize/main.cpp

main: ../cacheLineSize/mainQuiet ../cacheSize/mainQuiet main.cpp
	g++ -D CACHE_LINE_SIZE=$(shell ../cacheLineSize/mainQuiet) -D CACHE_SIZE=$(shell ../cacheSize/mainQuiet) -o main main.cpp
