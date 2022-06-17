_Pragma("once");

#include <signal.h>
#include <time.h>
#include <cstring>

#include <functional>

class timer
{
public:
    // 可以通过让TIMER_CALLBACK返回false,取消当前timer
    using TIMER_CALLBACK = std::function<bool(void*)>;
    
private:
    struct timer_context_t
    {
        timer_context_t()
        {
            memset(&sev, 0, sizeof(struct sigevent));
            memset(&trigger, 0, sizeof(struct itimerspec));

            sev.sigev_notify = SIGEV_THREAD;
            sev.sigev_notify_function = &thread_handler;

            trigger.it_interval.tv_sec = 1;// 确保周期性触发
        }
        ~timer_context_t()
        {
            if (timerid != nullptr)
            {
                timer_delete(timerid);
            }
        }

        timer_t timerid = nullptr;

        struct sigevent sev;
        struct itimerspec trigger;
        void *user_data = nullptr;
        TIMER_CALLBACK callback = nullptr;
    };
public:
    static timer_context_t *set(int micro_seconds, void *user_data, TIMER_CALLBACK callback)
    {
        timer_t timerid;
        timer_context_t *timer_context = new timer_context_t();
        timer_context->callback = callback;
        timer_context->user_data = user_data;

        timer_context->trigger.it_value.tv_sec = micro_seconds / 1000000;
        timer_context->trigger.it_value.tv_nsec = (micro_seconds % 1000000) * 1000;
        timer_context->sev.sigev_value.sival_ptr = timer_context;
        if (timer_create(CLOCK_REALTIME, &(timer_context->sev), &timerid) != 0)
        {
            return nullptr;
        }
        timer_context->timerid = timerid;

        if (timer_settime(timerid, 0, &(timer_context->trigger), NULL) != 0)
        {
            delete timer_context;
            return nullptr;
        }
        return timer_context;
    }

    using timer_handle = timer_context_t *;
    static void cancel(timer_handle timer_context)
    {
        timer_delete(timer_context->timerid);
        timer_context->timerid = nullptr;
        delete timer_context;
    }

private:
    static void thread_handler(union sigval sv)
    {
        timer_context_t *timer_context = reinterpret_cast<timer_context_t *>(sv.sival_ptr);

        if (!timer_context->callback(timer_context->user_data))
        {
            cancel(timer_context);
        }
    }
};