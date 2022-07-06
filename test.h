_Pragma("once");
#include "message_channel.hpp"

#include <boost/format.hpp>

#include <sys/time.h>
struct location
{
    location(int _x, int _y)
        : x(_x), y(_y)
    {
    }

    int x = 0;
    int y = 0;
};

using monitoring_channel = message_channel<bool, int, double, boost::interprocess::string, location>;

namespace
{
    const std::string monitoring_channel_name = "monitoring_channel";
    const int domain_id = 1024;

    std::string print_msg(const monitoring_channel::message &msg)
    {
        std::string val_str;
        switch (msg.value.index())
        {
        case 0:
            val_str = std::to_string(std::get<bool>(msg.value)) ;
            break;
        case 1:
            val_str = std::to_string(std::get<int>(msg.value));
            break;
        case 2:
            val_str = std::to_string(std::get<double>(msg.value));
            break;
        case 3:
            val_str = std::get<boost::interprocess::string>(msg.value);
            break;
        case 4:
            location loc = std::get<location>(msg.value);
            val_str = "x: " + std::to_string(loc.x) + " y: " + std::to_string(loc.y);
        }

        time_t t = msg.timestamp.tv_sec;
        tm *tm = localtime(&t);
        return (boost::format("%d-%d-%d %d:%d:%d.%ld %s: %s") 
        % (tm->tm_year + 1900)
        % (tm->tm_mon + 1)
        % tm->tm_mday
        % tm->tm_hour
        % tm->tm_min
        % tm->tm_sec
        % (msg.timestamp.tv_usec)

        % msg.name % val_str).str();
    }
}
