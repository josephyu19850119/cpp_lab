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

    optional<thread> lastInTask = UniOS::Task::Launch([]()
    {
        // 确保lastInTask和firstInTask在开始执行测试代码前，都进入可调度状态
        int ret = pthread_barrier_wait(&barrier);
        // lastInTask率先放弃可能先获得的时间片，确保firstInTask先占据CPU
        this_thread::yield();
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        for (int i = 0; i < iteration_count; ++i)
        {
            ++n;
        }
    },  UniOS::Task::TaskPriority::P0, true, // lastInTask和firstInTask优先级一样，且都为独占调用方式
        UniOS::Task::CpuSet("00000001")// 确保lastInTask和firstInTask运行在同一个CPU上以验证效果
    );

    optional<thread> firstInTask = UniOS::Task::Launch([]()
    {
        // 确保lastInTask和firstInTask在开始执行测试代码前，都进入可调度状态
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
    }, UniOS::Task::TaskPriority::P0, true, // lastInTask和firstInTask优先级一样，且都为独占调用方式
        UniOS::Task::CpuSet("00000001") // 确保lastInTask和firstInTask运行在同一个CPU上以验证效果
    );

    lastInTask.value().join();
    firstInTask.value().join();

    if (pthread_barrier_destroy(&barrier) != 0)
    {
        cerr << "pthread_barrier_destroy FAILED" << endl;
        exit(-1);
    }

    // Expected output 99999999
    cout << n << endl;
    return 0;
}