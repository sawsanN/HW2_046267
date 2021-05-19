# 046267 Computer Architecture - Spring 2020 - HW #2

# Environment for C++ 
CXX = g++
CXXFLAGS = -std=c++11 -Wall
CCLINK = $(CXX)
OBJS = cacheSim.o
#RM = rm -f
cacheSim: $(OBJS)
	$(CCLINK) -o cacheSim $(OBJS)

cacheSim.o : cacheSim.cpp cacheSim.h

.PHONY: clean
clean:
	rm -f *.o
	rm -f cacheSim
