#include "message_channel.hpp"

#include <iostream>
using namespace std;

int main()
{
    cout << boolalpha << boost::interprocess::shared_memory_object::remove(domain_shared_memory_name_formatter(1024).c_str()) << endl;
    
    return 0;
}