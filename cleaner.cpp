#include "test.h"

#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: msg_chan_cleaner [domain id] [channel name]" << endl;
        return 0;
    }

    int domain_id = stoi(argv[1]);
    string channel_name;
    if (argc > 2)
    {
        channel_name = argv[2];
    }

    if (monitoring_channel::remove(domain_id, channel_name))
    {
        if (channel_name.empty())
        {
            cout << "Removed shared memory of domain: " << domain_id << endl;
        }
        else
        {
            cout << "Removed channel: \"" << channel_name << "\" from shared memory of domain: " << domain_id << endl;
        }
    }
    else
    {
        if (channel_name.empty())
        {
            cout << "Cannot find shared memory of domain: " << domain_id << endl;
        }
        else
        {
            cout << "Cannot find channel: \"" << channel_name << "\" from shared memory of domain: " << domain_id << endl;
        }
    }

    return 0;
}