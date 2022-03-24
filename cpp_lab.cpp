#include <iostream>

using namespace std;

#include <boost/version.hpp>
#include <boost/format.hpp>

#include <sys/time.h>

int main(int argc, char **argv)
{
    cout << "Hello C++ lab" << endl;

    cout << "C++ std: " << __cplusplus << endl;
    cout << "boost version: " << BOOST_LIB_VERSION << endl;

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    struct tm *tm = localtime(&(tv.tv_sec));

    cout << (boost::format("%02d:%02d:%02d.%03d") % tm->tm_hour % tm->tm_min % tm->tm_sec % (tv.tv_usec / 1000)).str() << endl;
    return 0;
}