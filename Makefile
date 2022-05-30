cpp_lab : cpp_lab.cpp unios_task.h unios_task.cpp
	g++ -std=c++17 -pthread cpp_lab.cpp unios_task.cpp -o cpp_lab
clean :
	rm cpp_lab