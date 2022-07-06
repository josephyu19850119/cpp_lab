#include "message_channel.hpp"
#include "test.h"

#include <iostream>
#include <thread>
using namespace std;

int main()
{
    optional<monitoring_channel::reader> monitor = monitoring_channel::open_reader(domain_id, monitoring_channel_name);
    if (!monitor)
    {
        cout << "Not found domain shared memory: " << domain_shared_memory_name_formatter(domain_id) << endl;
        return 1;
    }

    while (true)
    {
        vector<monitoring_channel::message> msgs;

        if (!monitor.value().read(msgs))
        {
            cout << monitor.value().get_err_text() << endl;
            break;
        }
        else if (msgs.empty())
        {
            this_thread::yield();
            continue;
        }

        for (const monitoring_channel::message &msg : msgs)
        {
            cout << print_msg(msg) << endl;
        }
    }

    return 0;
}