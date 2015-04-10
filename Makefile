all: test
 
clean:
	rm test
	 
#g++-4.8 -fopenmp -O0 -Wall -stdlib=libc++ -std=c++11 -o test test.cpp
test: test.cpp factorUtil.h
	g++ -O0 -Wall -std=c++11 -o test test.cpp


 
