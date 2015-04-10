all: test
 
clean:
	rm test
	 
test: test.cpp factorUtil.h
	g++ -O0 -Wall -std=c++11 -o test.bin test.cpp


 
