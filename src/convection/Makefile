INCLUDE = -I../../
INCLUDE += -I$(HOME)/include

CFLAGS = -Wall -std=c++11 -fopenmp
LIBS = $$(pkg-config --static --libs glfw3) -lGL -lfftw3
CXX = g++

all: convection

convection: convection.cpp convection_solver.h 
	$(CXX) -O3 $< $(CFLAGS) $(INCLUDE) $(LIBS) -o $@

clean:
	rm -f convection 
