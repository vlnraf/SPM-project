CXX = g++
CXXFLAGS = -std=c++14 -Wall
LDFLAGS = -pthread

all: main

main: main.o graph.o
	$(CXX) $(CXXFLAGS) -o main main.o graph.o

out/graph.o: graph.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c graph.cpp

main.o: main.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f main main.o graph.o
