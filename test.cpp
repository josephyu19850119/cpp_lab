#include "recording_pipeline.hpp"

#include <iostream>
using namespace std;

int main()
{
    using pipe_t = recording_pipe<int, double, bool, boost::interprocess::string>;
    pipe_t::writer sender = pipe_t::open_writer(0, "test");

    sender.send(pipe_t::record("int", 777));
    sender.send(pipe_t::record("double", 3.1415926));
    // sender.send(pipe_t::record("bool", true));
    // sender.send(pipe_t::record("text", "oolololoolo"));

    pipe_t::reader receiver = pipe_t::open_reader(0, "test");
    std::vector<pipe_t::record> records = receiver.read();
    for (const pipe_t::record &rec : records)
    {
        std::cout << rec.name << ": ";
        switch (rec.value.index())
        {
        case 0:
            std::cout << std::get<int>(rec.value) << std::endl;
            break;
        case 1:
            std::cout << std::get<double>(rec.value) << std::endl;
            break;
        case 2:
            std::cout << std::get<bool>(rec.value) << std::endl;
            break;
        case 3:
            std::cout << std::get<boost::interprocess::string>(rec.value) << std::endl;
            break;
        }
    }


    cout << "----------------------" << endl;
    
    sender.send(pipe_t::record("bool", true));
    sender.send(pipe_t::record("text", boost::interprocess::string("oolololoolo")));

     records = receiver.read();
    for (const pipe_t::record &rec : records)
    {
        std::cout << rec.name << ": ";
        switch (rec.value.index())
        {
        case 0:
            std::cout << std::get<int>(rec.value) << std::endl;
            break;
        case 1:
            std::cout << std::get<double>(rec.value) << std::endl;
            break;
        case 2:
            std::cout << std::get<bool>(rec.value) << std::endl;
            break;
        case 3:
            std::cout << std::get<boost::interprocess::string>(rec.value) << std::endl;
            break;
        }
    }

    return 0;
}