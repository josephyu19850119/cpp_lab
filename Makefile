cpp_lab : test.cpp timer.hpp
	g++ -std=c++11 -DBOOST_LOG_DYN_LINK test.cpp -lrt -lboost_log -lboost_log_setup -lboost_thread -lpthread -lboost_filesystem -o cpp_lab
clean :
	rm cpp_lab