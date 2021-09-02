CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3
LDFLAGS = -pthread
FASTFLOW := libraries/fastflow

BIN := bin
SRC := src
INCLUDE := -I include
FFINCLUDE := -I $(FASTFLOW)

EXECUTABLE := main
BFS := bfs

all: $(BIN)/graph_search $(BIN)/ff_graph_search

$(BIN)/graph_search: $(SRC)/graph.cpp $(SRC)/graph_search.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

$(BIN)/ff_graph_search: $(SRC)/graph.cpp $(SRC)/ff_graph_search.cpp
	$(CXX) $(CXXFLAGS) $(FFINCLUDE) $(INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(BIN)/*
