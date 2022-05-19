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

    thread first_in_first_out_thread([]() {
        // 确保first_in_first_out_thread, round_robin_thread两个线程竞争同一个CPU,以验证线程schedule policy的效果
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(0, &cpu_set);
        if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
        {
            cerr << "sched_setaffinity FAILED" << endl;
            exit(-1);
        }

        // 将first_in_first_out_thread设置为Realtime线程中first in first out独占调度策略，且为最低优先级
        sched_param parma;
        parma.sched_priority = sched_get_priority_min(SCHED_FIFO);
        if (parma.sched_priority == -1)
        {
            cerr << "sched_get_priority_min FAILED" << endl;
            exit(-1);
        }

        if (sched_setscheduler(0, SCHED_FIFO, &parma) != 0)
        {
            
            cerr << "sched_setscheduler FAILED" << endl;
            exit(-1);
        }

        // 等待round_robin_thread线程属性设置完成后再执行后续操作
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }

        // 两个线程属性都设置完成，开始执行测试代码
        // first_in_first_out_thread独占CPU,不会被round_robin_thread的反复正负翻转所干扰，把var连续累加至1000000          
        int count = 0;
        while (count++ < 999999)
        {
            ++var;
        } });

    thread round_robin_thread([]() {
        // 确保first_in_first_out_thread, round_robin_thread两个线程竞争同一个CPU,以验证schedule policy的效果
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(0, &cpu_set);
        if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
        {
            cerr << "sched_setaffinity FAILED" << endl;
            exit(-1);
        }

        // 将round_robin_thread设置为Realtime线程中非独占的round_robin轮询调度策略，且同样为最低优先级
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

        // 等待first_in_first_out_thread线程属性设置完成后再执行后续操作
        int ret = pthread_barrier_wait(&barrier);
        if (ret != PTHREAD_BARRIER_SERIAL_THREAD && ret != 0)
        {
            cerr << "pthread_barrier_wait FAILED" << endl;
            exit(-1);
        }
        // 放弃可能先于first_in_first_out_thread获得的时间片，确保first_in_first_out_thread先占据CPU
        std::this_thread::yield();

        // 两个线程属性都设置完成，开始执行测试代码
        // 反复翻转var正负号，但只会在first_in_first_out_thread完全执行完后才会开始，所以最后改变的只是var的正负号，而对绝对值没有影响
        int count  = 0;
        while (count++ < 999999)
        {
            var = -var;
        } });

    first_in_first_out_thread.join();
    round_robin_thread.join();

    if (pthread_barrier_destroy(&barrier) != 0)
    {
        cerr << "pthread_barrier_destroy FAILED" << endl;
        exit(-1);
    }

    // Expected output -1000000
    cout << var << endl;
    return 0;
}