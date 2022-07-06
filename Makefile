test : message_channel.hpp test.cpp Makefile
	g++ -std=c++17 -g -Wall test.cpp -pthread -lrt -o test
clean :
	rm test