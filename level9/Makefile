CXX = g++
CXXFLAGS = -std=c++17 -O2 -pthread

PROG_SRC = implement_proactor.cpp
PROG_OBJ = $(PROG_SRC:.cpp=.o)
PROG = $(PROG_SRC:.cpp=)

LIB = proactor
LIB_SRC = ../level8/proactor.cpp
LIB_OBJ = $(LIB_SRC:.cpp=.o)
LIB_FILE = lib$(LIB).a

all: $(PROG)

$(LIB_FILE): $(LIB_OBJ)
	ar -rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PROG): $(LIB_FILE) $(PROG_OBJ)
	$(CXX) $(CXXFLAGS) $(PROG_OBJ) -L. -l$(LIB) -o $@

clean:
	rm -f $(PROG) $(LIB_FILE) $(PROG_OBJ) $(LIB_OBJ)
