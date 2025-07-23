CXX = g++
CXXFLAGS = -std=c++17 -O2

TARGET = convex_interactive
SRC = convex_interactive.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
