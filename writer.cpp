#include "message_channel.hpp"
#include "test.h"

#include <iostream>
#include <vector>
using namespace std;

int main()
{
    monitoring_channel::writer recorder = monitoring_channel::open_writer(domain_id, monitoring_channel_name);

    vector<monitoring_channel::message> msgs{
        monitoring_channel::message("ok", true),
        monitoring_channel::message("speed", 60.1),
        monitoring_channel::message("level", -1),
        monitoring_channel::message("info", boost::interprocess::string("I'm OK")),
        monitoring_channel::message("location", location(3, 7))};

    for(const monitoring_channel::message& msg : msgs)
    {
        getchar();
        cout << print_msg(msg) << endl;
        recorder.write(msg);
    }
    return 0;
};