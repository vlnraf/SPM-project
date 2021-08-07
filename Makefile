CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3
LDFLAGS = -pthread

BIN := bin
SRC := src
INCLUDE := -I include

EXECUTABLE := main
BFS := bfs

all: $(BIN)/graph_search

$(BIN)/graph_search: $(SRC)/graph.cpp $(SRC)/graph_search.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)


clean:
	rm -f $(BIN)/*
