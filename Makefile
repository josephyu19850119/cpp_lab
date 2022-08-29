cpp_lab : cpp_lab.cpp
	g++ -std=c++17 cpp_lab.cpp -lfastdds_statistics_backend -o cpp_lab
clean :
	rm cpp_lab