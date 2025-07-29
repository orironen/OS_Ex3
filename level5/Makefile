CXX = g++
CXXFLAGS = -std=c++17 -O2 -pthread

TARGET = reactor
SRC = reactor.cpp

all: $(TARGET)

$(TARGET): $(SRC)
    $(CXX) $(CXXFLAGS) -o $@ $<

clean:
    rm -f $(TARGET)