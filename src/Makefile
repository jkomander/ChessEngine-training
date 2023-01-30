# Project name
PROJECT = training_data_loader

# Libraray name
LIB = $(PROJECT).so

# Source files
SRC = training_data_loader.cpp

# Object files
OBJS = $(subst .cpp,.o,$(SRC))

# High-level configuration
debug = no
optimize = yes

# Low-level configuration
COMP = gcc
CXX = g++
CXXFLAGS = -std=c++17 -fPIC
LDFLAGS = -shared

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
	$(CXX) $(CXXFLAGS) -o $(LIB) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(LIB) *.o chess/*.o