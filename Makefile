CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3
LDFLAGS = -pthread -fopenmp

BIN := bin
SRC := src
INCLUDE := -I include

EXECUTABLE := main
BFS := bfs

all: $(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)


clean:
	rm -f $(BIN)/*
