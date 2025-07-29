CXX = g++
CXXFLAGS = -std=c++17 -O2 -pthread

TARGET = implement_reactor
SRC = implement_reactor.cpp ../level5/reactor.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
