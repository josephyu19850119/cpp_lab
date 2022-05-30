#include "unios_task.h"

#include <iostream>
#include <atomic>
#include <cassert>
using namespace std;

atomic_int n{0};
pthread_barrier_t barrier;
const int iteration_count = 100000000;

optional<thread> lowPriorityTask;
optional<thread> highPriorityTask;
optional<thread> highSubTaskInLowPriorityTask;
optional<thread> lowSubTaskInHighPriorityTask;

int main(int argc, char **argv)
{
    if (pthread_barrier_init(&barrier, nullptr, 4) != 0)
    {
        cerr << "pthread_barrier_init FAILED" << endl;
        exit(-1);
    }

    lowPriorityTask = UniOS::Task::Launch([]()
    {
        timespec tp;
        if (sched_rr_get_interval(0, &tp) != 0)
        {
            cerr << "sched_rr_get_interval FAILED" << endl;
            exit(-1);
        }
        cout << "In lowPriorityTask, tp.tv_sec: " << tp.tv_sec << "; tp.tv_nsec: " << tp.tv_nsec << endl;

        highSubTaskInLowPriorityTask = UniOS::Task::SubTask([]()
        {
            timespec tp;
            if (sched_rr_get_interval(0, &tp) != 0)
            {
                cerr << "sched_rr_get_interval FAILED" << endl;
                exit(-1);
            }
            cout << "In highSubTaskInLowPriorityTask, tp.tv_sec: " << tp.tv_sec << "; tp.tv_nsec: " << tp.tv_nsec << endl;

            // 确保4个待测试task在开始执行测试代码前，都进入可调度状态
            int ret = pthread_barrier_wait(&barrier);
            if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
            {
                cerr << "pthread_barrier_wait FAILED" << endl;
                exit(-1);
            }

            // highSubTaskInLowPriorityTask第三个运行，之前的lowSubTaskInHighPriorityTask将n设置为iteration_count / 2
            assert(n == iteration_count / 2);

            for (int i = 0; i < iteration_count; ++i)
            {
                --n;
            }
        }, UniOS::Task::SubTaskPriority::High4, false, UniOS::Task::CpuSet("00000001"));// 确保4个待测试task运行在同一个CPU上以验证效果

        // 确保4个待测试task在开始执行测试代码前，都进入可调度状态
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        // lowPriorityTask最后运行，之前的highSubTaskInLowPriorityTask将n设置为-(iteration_count / 2)
        assert(n == -(iteration_count / 2));

        for (int i = 0; i < iteration_count / 2; ++i)
        {
            --n;
        }
    },  UniOS::Task::TaskPriority::P0, false,
        UniOS::Task::CpuSet("00000001")// 确保4个待测试task运行在同一个CPU上以验证效果
    );

    highPriorityTask = UniOS::Task::Launch([]()
    {
        timespec tp;
        if (sched_rr_get_interval(0, &tp) != 0)
        {
            cerr << "sched_rr_get_interval FAILED" << endl;
            exit(-1);
        }
        cout << "In highPriorityTask, tp.tv_sec: " << tp.tv_sec << "; tp.tv_nsec: " << tp.tv_nsec << endl;

        lowSubTaskInHighPriorityTask = UniOS::Task::SubTask([]()
        {
            timespec tp;
            if (sched_rr_get_interval(0, &tp) != 0)
            {
                cerr << "sched_rr_get_interval FAILED" << endl;
                exit(-1);
            }
            cout << "In lowSubTaskInHighPriorityTask, tp.tv_sec: " << tp.tv_sec << "; tp.tv_nsec: " << tp.tv_nsec << endl;

            // 确保4个待测试task在开始执行测试代码前，都进入可调度状态
            int ret = pthread_barrier_wait(&barrier);
            if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
            {
                cerr << "pthread_barrier_wait FAILED" << endl;
                exit(-1);
            }

            // lowSubTaskInHighPriorityTask第二个运行，check n应该为highPriorityTask结束时的值
            assert(n == iteration_count);

            for (int i = 0; i < iteration_count / 2; ++i)
            {
                --n;
            }
        }, UniOS::Task::SubTaskPriority::Low4, false, UniOS::Task::CpuSet("00000001"));// 确保4个待测试task运行在同一个CPU上以验证效果

        // 确保4个待测试task在开始执行测试代码前，都进入可调度状态
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        // highPriorityTask最先开始运行，check n应该为初始零值
        assert(n == 0);

        for (int i = 0; i < iteration_count; ++i)
        {
            ++n;
        }
    }, UniOS::Task::TaskPriority::P1, false,
        UniOS::Task::CpuSet("00000001") // 确保4个待测试task运行在同一个CPU上以验证效果
    );

    lowPriorityTask.value().join();
    highPriorityTask.value().join();
    highSubTaskInLowPriorityTask.value().join();
    lowSubTaskInHighPriorityTask.value().join();

    if (pthread_barrier_destroy(&barrier) != 0)
    {
        cerr << "pthread_barrier_destroy FAILED" << endl;
        exit(-1);
    }

    assert(n == -iteration_count);

    cout << "Test Pass" << endl;
    return 0;
}