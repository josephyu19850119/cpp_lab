cpp_lab : cpp_lab.cpp
	g++ -std=c++11 cpp_lab.cpp -lncurses -o cpp_lab
clean :
	rm cpp_lab