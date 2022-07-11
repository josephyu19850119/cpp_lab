msg_writer : message_channel.hpp writer.cpp test.h Makefile
	g++ -std=c++17 -g -Wall writer.cpp -pthread -lrt -o msg_writer
msg_reader : message_channel.hpp reader.cpp test.h Makefile
	g++ -std=c++17 -g -Wall reader.cpp -pthread -lrt -o msg_reader
msg_chan_dumper : message_channel.hpp dumper.cpp test.h Makefile
	g++ -std=c++17 -g -Wall dumper.cpp -pthread -lrt -o msg_chan_dumper
msg_chan_cleaner : message_channel.hpp cleaner.cpp test.h Makefile
	g++ -std=c++17 -g -Wall cleaner.cpp -pthread -lrt -o msg_chan_cleaner
clean :
	rm test