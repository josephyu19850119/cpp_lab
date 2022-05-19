#include <iostream>
#include <thread>
#include <atomic>
using namespace std;

#include <pthread.h>
#include <sched.h>

atomic_int var{1};
pthread_barrier_t barrier;

int main(int argc, char **argv)
{
    if (pthread_barrier_init(&barrier, nullptr, 2) != 0)
    {
        cerr << "pthread_barrier_init FAILED" << endl;
        exit(-1);
    }

    thread realtime_thread([]() {
        // 确保realtime_thread, other_thread两个线程竞争同一个CPU,以验证线程优先级的效果
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(0, &cpu_set);
        if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
        {
            cerr << "sched_setaffinity FAILED" << endl;
            exit(-1);
        }

        // 将realtime_thread设置为Realtime线程中非独占的round_robin轮询调度策略，且优先级最低
        sched_param parma;
        parma.sched_priority = sched_get_priority_min(SCHED_RR);
        if (parma.sched_priority == -1)
        {
            cerr << "sched_get_priority_min FAILED" << endl;
            exit(-1);
        }

        if (sched_setscheduler(0, SCHED_RR, &parma) != 0)
        {
            
            cerr << "sched_setscheduler FAILED" << endl;
            exit(-1);
        }

        // 等待other_thread线程属性设置完成后再执行后续操作
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        // 两个线程属性都设置完成，开始执行测试代码
        // 即使如上述设置，也完全不会被非Realtime线程哪怕是暂时的抢占，把var连续累加至1000000
        // 不会被other_thread的反复正负翻转所干扰          
        int count = 0;
        while (count++ < 999999)
        {
            ++var;
        } });

    thread other_thread([]() {
        // 确保realtime_thread, other_thread两个线程竞争同一个CPU,以验证线程优先级的效果
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(0, &cpu_set);
        if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
        {
            cerr << "sched_setaffinity FAILED" << endl;
            exit(-1);
        }

        // 等待realtime_thread线程属性设置完成后再执行后续操作
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        // 两个线程属性都设置完成，开始执行测试代码
        // 反复翻转var正负号，但只会在realtime_thread完全执行完后才会开始，所以最后改变的只是var的正负号，而对绝对值没有影响
        int count  = 0;
        while (count++ < 999999)
        {
            var = -var;
        } });

    realtime_thread.join();
    other_thread.join();

    if (pthread_barrier_destroy(&barrier) != 0)
    {
        cerr << "pthread_barrier_destroy FAILED" << endl;
        exit(-1);
    }

    // Expected ouput -1000000
    cout << var << endl;
    return 0;
}