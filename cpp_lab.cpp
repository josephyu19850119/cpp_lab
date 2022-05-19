#include <iostream>
#include <thread>
#include <atomic>
using namespace std;

#include <sched.h>

atomic_int var{1};

int main(int argc, char **argv)
{
    thread t0([]() {
        // 确保t0, t1两个线程竞争同一个CPU,以验证线程优先级的效果
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(0, &cpu_set);
        if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
        {
            cerr << "sched_setaffinity FAILED" << endl;
            exit(-1);
        }

        // 将t0设置为Realtime线程中非独占的round_robin轮询调度策略，且优先级最低
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

        // 即使如上述设置，也完全不会被非Realtime线程哪怕是暂时的抢占，把var连续累加至1000000
        // 不会被t1的反复正负翻转所干扰          
        int count = 0;
        while (count++ < 999999)
        {
            ++var;
        } });

    thread t1([]() {
        // 确保t0, t1两个线程竞争同一个CPU,以验证线程优先级的效果
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(0, &cpu_set);
        if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
        {
            cerr << "sched_setaffinity FAILED" << endl;
            exit(-1);
        }

        // 反复翻转var正负号，但只会在t0完全执行完后才会开始，所以最后改变的只是var的正负号，而对绝对值没有影响
        int count  = 0;
        while (count++ < 999999)
        {
            var = -var;
        } });

    t0.join();
    t1.join();

    cout << var << endl;
    return 0;
}