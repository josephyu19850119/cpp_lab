writer : message_channel.hpp writer.cpp test.h Makefile
	g++ -std=c++17 -g -Wall writer.cpp -pthread -lrt -o writer
reader : message_channel.hpp reader.cpp test.h Makefile
	g++ -std=c++17 -g -Wall reader.cpp -pthread -lrt -o reader
cleaner : message_channel.hpp cleaner.cpp test.h Makefile
	g++ -std=c++17 -g -Wall cleaner.cpp -pthread -lrt -o cleaner
clean :
	rm test