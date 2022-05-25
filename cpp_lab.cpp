#include "unios_task.h"

#include <pthread.h>

#include <iostream>
#include <atomic>
using namespace std;

atomic_int n{1};
pthread_barrier_t barrier;
const int iteration_count = 100000000;

int main(int argc, char **argv)
{
    if (pthread_barrier_init(&barrier, nullptr, 2) != 0)
    {
        cerr << "pthread_barrier_init FAILED" << endl;
        exit(-1);
    }

    optional<thread> lowPriorityTask = UniOS::Task::Launch([]()
    {
        // 确保lowPriorityTask和highPriorityTask在开始执行测试代码前，都进入可调度状态
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        for (int i = 0; i < iteration_count; ++i)
        {
            ++n;
        }
    },  UniOS::Task::TaskPriority::P0, false,
        UniOS::Task::CpuSet("00000001")// 确保lowPriorityTask和highPriorityTask运行在同一个CPU上以验证效果
    );

    optional<thread> highPriorityTask = UniOS::Task::Launch([]()
    {
        // 确保lowPriorityTask和highPriorityTask在开始执行测试代码前，都进入可调度状态
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        for (int i = 0; i < iteration_count - 1; ++i)
        {
            n = -n;
        }
    }, UniOS::Task::TaskPriority::P1, false,
        UniOS::Task::CpuSet("00000001") // 确保lowPriorityTask和highPriorityTask运行在同一个CPU上以验证效果
    );

    lowPriorityTask.value().join();
    highPriorityTask.value().join();

    if (pthread_barrier_destroy(&barrier) != 0)
    {
        cerr << "pthread_barrier_destroy FAILED" << endl;
        exit(-1);
    }

    // Expected output 99999999
    cout << n << endl;
    return 0;
}