CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
TARGET = programa
SRC = crp.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean