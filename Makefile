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

all: $(BIN)/cpp_graph_search $(BIN)/ff_graph_search $(BIN)/graph_generator $(BIN)/sequential_graph_search

debug: $(BIN)/cpp_graph_search_d $(BIN)/ff_graph_search_d

$(BIN)/cpp_graph_search: $(SRC)/graph.cpp $(SRC)/cpp_graph_search.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

$(BIN)/ff_graph_search: $(SRC)/graph.cpp $(SRC)/ff_graph_search.cpp
	$(CXX) $(CXXFLAGS) $(FFINCLUDE) $(INCLUDE) $^ -o $@ $(LDFLAGS)

$(BIN)/sequential_graph_search: $(SRC)/graph.cpp $(SRC)/sequential_graph_search.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ 

$(BIN)/graph_generator: $(SRC)/graph_generator.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

$(BIN)/cpp_graph_search_d: $(SRC)/graph.cpp $(SRC)/cpp_graph_search.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS) -DDEBUG

$(BIN)/ff_graph_search_d: $(SRC)/graph.cpp $(SRC)/ff_graph_search.cpp
	$(CXX) $(CXXFLAGS) $(FFINCLUDE) $(INCLUDE) $^ -o $@ $(LDFLAGS) -DDEBUG

clean:
	rm -f $(BIN)/*
