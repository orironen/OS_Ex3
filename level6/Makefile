CXX = g++
CXXFLAGS = -std=c++17 -O2

SRC = implement_reactor.cpp
TARGET = $(SRC:.cpp=)


all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
