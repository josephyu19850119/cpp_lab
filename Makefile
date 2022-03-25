cpp_lab : cpp_lab.cpp func.h func.cpp logging.hpp
	g++ -std=c++11 -DBOOST_LOG_DYN_LINK cpp_lab.cpp func.cpp -lboost_log -lboost_log_setup -lboost_thread -lpthread -lboost_filesystem -o cpp_lab
clean :
	rm cpp_lab