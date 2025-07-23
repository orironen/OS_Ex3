CXX = g++
CXXFLAGS = -std=c++17 -O2

TARGET = CH
SRC = CH.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
