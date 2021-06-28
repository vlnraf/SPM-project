CXX = g++
CXXFLAGS = -std=c++14 -Wall
LDFLAGS = -pthread

BIN := bin
SRC := src
INCLUDE := -I include

EXECUTABLE := main

all: $(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@


clean:
	rm -f $(BIN)/*
