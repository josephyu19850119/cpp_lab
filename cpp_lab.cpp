#include <iostream>

using namespace std;

#include <boost/version.hpp>

int main(int argc, char **argv)
{
    cout << "Hello C++ lab" << endl;

    cout << "C++ std: " << __cplusplus << endl;
    cout << "boost version: " << BOOST_LIB_VERSION << endl;

    return 0;
}