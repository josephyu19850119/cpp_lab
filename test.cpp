#include <chrono>
#include <thread>
using namespace std;

#include <boost/log/trivial.hpp>

#include "timer.hpp"

int main(int argc, char **argv)
{
    int count = 0;
    // 每1000微秒(0.001秒)回调一次，1000次后取消
    timer::timer_handle t = timer::set(chrono::duration<int, micro>(1000), &count, [](void *p) { 
        int* count = static_cast<int*>(p);
        BOOST_LOG_TRIVIAL(info) << "tick tock: " << ++(*count);
        return true; });

    while (count < 100)
    {
        std::this_thread::yield();
    }

    timer::cancel(t);

    // 每1000微秒(0.001秒)回调一次，count降为0时主动停止
    t = timer::set(chrono::duration<int, micro>(1000), &count, [](void *p) {
        int* count = static_cast<int*>(p);
        BOOST_LOG_TRIVIAL(info) << "tick tock: " << (*count)--;
        if (*count == 0) {
            return false;
        }
        return true; });

    pause();
    return 0;
}