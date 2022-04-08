cpp_lab : test.cpp timer.hpp
	g++ -std=c++17 -DBOOST_LOG_DYN_LINK test.cpp -lboost_log -lboost_log_setup -lboost_thread -lpthread -lboost_filesystem -o timer_test
clean :
	rm timer_test