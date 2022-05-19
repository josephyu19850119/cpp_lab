cpp_lab : cpp_lab.cpp
	g++ -std=c++11 -pthread cpp_lab.cpp -o cpp_lab
clean :
	rm cpp_lab