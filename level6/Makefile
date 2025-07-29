CXX = g++
<<<<<<< HEAD
CXXFLAGS = -std=c++17 -O2

SRC = implement_reactor.cpp
TARGET = $(SRC:.cpp=)

=======
CXXFLAGS = -std=c++17 -O2 -pthread

TARGET = implement_reactor
SRC = implemet_reactor.cpp ../level5/reactor.cpp
>>>>>>> ee1ff0f (implemented reactor pattern)

all: $(TARGET)

$(TARGET): $(SRC)
<<<<<<< HEAD
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
=======
    $(CXX) $(CXXFLAGS) -o $@ $^

clean:
    rm -f $(TARGET)
>>>>>>> ee1ff0f (implemented reactor pattern)
