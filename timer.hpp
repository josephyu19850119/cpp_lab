#include <chrono>
#include <thread>
#include <atomic>
#include <boost/log/trivial.hpp>

// 可以通过让TIMER_CALLBACK返回false,取消当前timer
typedef bool (*TIMER_CALLBACK)(void *data);

class timer
{
public:
    void set(std::chrono::duration<int, std::micro> interval, void *user_data, TIMER_CALLBACK callback)
    {
        std::thread([=]
                    { 
                        while(!this->m_canceled) 
                        {
                            std::this_thread::sleep_for(interval);
                            if (callback(user_data) == false)
                            {
                                break;
                            };
                        } })
            .detach();

        m_canceled = false;
    }

    void cancel()
    {
        m_canceled = true;
    }

    ~timer()
    {
        m_canceled = true;
    }

private:
    std::atomic_bool m_canceled;
};