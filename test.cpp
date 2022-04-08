
#include <unistd.h>

#include <iostream>
#include <thread>
#include <atomic>

#include <boost/log/trivial.hpp>

#include "timer.hpp"

using namespace std;

bool callback(void *user_data)
{
    atomic_int *pi = reinterpret_cast<atomic_int *>(user_data);

    BOOST_LOG_TRIVIAL(info) << "Callback " << ++(*pi) << " times";
    return true;
}

int main(void)
{
    // 先调用5次后取消
    atomic_int count{0};
    timer::timer_context_t *t = timer::set(1, 0, &count, callback);

    while (count < 5)
    {
        sleep(1);
    }

    timer::cancel(t);

    // 再调用5次后自行结束
    t = timer::set(1, 0, &count, [](void *p)
                   {

        int* i = reinterpret_cast<int*>(p);
        BOOST_LOG_TRIVIAL(info) << "count in callback lambda: " << (*i)--;
        return *i > 0; });

    pause();
}