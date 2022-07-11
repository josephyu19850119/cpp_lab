#include "test.h"
#include "message_channel.hpp"

#include <string>
using namespace std;

int main(int argc, char *argv[])
{
    string dump_path;
    if (argc > 1)
    {
        dump_path = argv[1];
    }

    monitoring_channel::dump_message_channel(domain_id, monitoring_channel_name, dump_path);

    return 0;
}