_Pragma("once");

#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

#include <unistd.h>

class timer
{
    class timer_context
    {
        friend timer;
        std::atomic_bool canceled{false};
    };

public:
    using timer_handle = timer_context *;

    // 可以通过让TIMER_CALLBACK返回false,取消当前timer
    using TIMER_CALLBACK = std::function<bool(void *)>;

    static timer_handle set(useconds_t  micro_seconds, void *user_data, TIMER_CALLBACK callback)
    {
        timer_handle t = new timer_context();
        std::thread([=] { 
                        while(!t->canceled) 
                        {
                            usleep(micro_seconds);
                            if (callback(user_data) == false)
                            {
                                break;
                            };
                        } })
            .detach();

        return t;
    }

    static void cancel(timer_handle t)
    {
        t->canceled = true;
    }
};