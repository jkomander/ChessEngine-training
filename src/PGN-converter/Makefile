# Project name
PROJECT = pgn_converter

# Executable name
EXE = $(PROJECT)

ifeq ($(OS),Windows_NT)
	EXE += $(.exe)
endif

# Source files
SRC = main.cpp

# Object files
OBJS = $(subst .cpp,.o,$(SRC))

# High-level configuration
debug = no
optimize = yes

# Low-level configuration
COMP = gcc
CXX = g++
CXXFLAGS = -std=c++17

# Debugging
ifeq ($(debug),no)
	CXXFLAGS += -DNDEBUG
else
	CXXFLAGS += -g
endif

# Optimization
ifeq ($(optimize),yes)
	CXXFLAGS += -O3
endif

# Targets
.PHONY: build clean

build: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(OBJS)

clean:
	rm -f $(EXE) *.o

depend: .depend

.depend: $(SRC)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

dist-clean: clean
	rm -f *~ .depend

include .depend