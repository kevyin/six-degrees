##
## Makefile for COMP3171/9171 Deliverable 1: Six Degrees
##

CXX = g++
#CPPFLAGS = -Wall -Werror -O2

# enable this for debugging
#CPPFLAGS = -Wall -g

default: imdb-test six-degrees

imdb-test: imdb.o imdb-test.o
	$(CXX) $(CPPFLAGS) -o imdb-test imdb.o imdb-test.o

six-degrees: imdb.o path.o six-degrees.o
	$(CXX) $(CPPFLAGS) -o six-degrees imdb.o path.o six-degrees.o

imdb-test.o: imdb-utils.h imdb-test.cpp
	$(CXX) $(CPPFLAGS) -c imdb-test.cpp

six-degrees.o: imdb.h imdb-utils.h path.h six-degrees.cpp
	$(CXX) $(CPPFLAGS) -c six-degrees.cpp
  
imdb.o: imdb.h imdb-utils.h imdb.cpp
	$(CXX) $(CPPFLAGS) -c imdb.cpp

path.o: path.h imdb-utils.h path.cpp
	$(CXX) $(CPPFLAGS) -c path.cpp

clean: 
	rm -rf *.o a.out core *.dSYM

immaculate: clean
	rm -f imdb-test six-degrees
