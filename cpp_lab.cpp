#include <chrono>
using namespace std;

#include <boost/log/trivial.hpp>

#include "timer.hpp"

int main(int argc, char **argv)
{
    timer t;
    int count = 0;
    t.set(chrono::duration<int, micro>(1000000), &count, [](void *p)
          { 
              int* count = static_cast<int*>(p);
              if (*count == 10) {
                  return false;
              }
              BOOST_LOG_TRIVIAL(info) << "tick tock: " << (*count)++;
              return true; });

    pause();
    return 0;
}