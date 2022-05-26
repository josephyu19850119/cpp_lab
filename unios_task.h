#include <sched.h>
#include <thread>
#include <bitset>
#include <optional>

namespace UniOS
{
    struct Task
    {
        // Task可有11个不同的优先级，P0最低，P10最高
        enum class TaskPriority
        {
            P0,
            P1,
            P2,
            P3,
            P4,
            P5,
            P6,
            P7,
            P8,
            P9,
            P10
        };

        // Task可以再开启SubTask,可有9个不同的优先级
        // SubTask优先级作用范围在所处Task优先级内，也就是说更高优先级Task内的最低SubTask,也比更低优先级Task内的最高SubTask更优先
        enum class SubTaskPriority
        {
            High4 = 4,
            High3 = 3,
            High2 = 2,
            High1 = 1,
            Regular = 0,
            Low2 = -2,
            Low1 = -1,
            Low3 = -3,
            Low4 = -4
        };

        // 用于限定Task只可以在哪些CPU上运行，目前最多支持在64个CPU上进行选择
        // 比如在8核CPU上指定第一个和第三个核心,可以用CpuSet("00000101")
        using CpuSet = std::bitset<64>;

        // 启动一个Task
        // worker: Task的运行代码，可以是任何可调用(_Callable)类型对象，包括function pointer、function objects、 std::function object，lambda表达式
        // priority：Task优先级，默认为最低P0
        // isExclusive: 是否为独占调度方式，如果是，CPU一旦分配给某一task线程，则不会在相同优先级Task之间轮询，直到任务完成或进入等待状态或被更高优先级任务抢占，默认为非独占
        // cpuSet: 限定在某些CPU上运行，默认所有CPU可用
        // args: 传给worker的可变参数列表
        // 返回: 如按预期创建thread，则在optional中被返回，否则返回空optional
        template <typename Worker, typename... Args>
        static std::optional<std::thread> Launch(Worker worker,
                           TaskPriority priority = TaskPriority::P0, bool isExclusive = false, CpuSet cpuSet = UINT64_MAX,
                           Args &&...args)
        {
            // Task只能被非task运行线程开启，task线程可以再进一步开启SubTask，但不能再开启Task
            if (workerContext.inTask != WorkerContext::InTask::OutTask)
            {
                return std::optional<std::thread>();
            }

            return std::thread([&worker, &args...](TaskPriority priority, bool isExclusive, CpuSet cpuSet)
            {
                // 如果限定了可用CPU，则进行相关设置
                if (!cpuSet.all())
                {
                    cpu_set_t cpu_set;
                    CPU_ZERO(&cpu_set);

                    for(std::size_t i = 0; i < cpuSet.size(); ++i)
                    {
                        if(cpuSet[i])
                        {
                            CPU_SET(i, &cpu_set);
                        }
                    }

                    if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
                    {
                        // TODO: return erro info to caller
                        return;
                    }
                }

                // 将任务与当前线程的关系、线程优先级、调度方式保存thread local storage中

                workerContext.inTask = WorkerContext::InTask::MainTask;
                // 根据priority参数设置线程的Linux定义的优先级，具体说明见https://linux.die.net/man/2/sched_setscheduler
                workerContext.systemPriorty = static_cast<int>(priority) * 9 + 5;
                workerContext.isExclusive = isExclusive;

                // 根据priority和isExclusive参数，设置Linux定义的优先级和调度策略，也参考上面链接
                sched_param priorityParma;
                priorityParma.sched_priority = workerContext.systemPriorty;

                if (sched_setscheduler(0, (isExclusive ? SCHED_FIFO : SCHED_RR), &priorityParma) != 0)
                {
                    // TODO: return erro info to caller
                    return;
                }

                // 开始运行task代码
                worker(args...);
            }, priority, isExclusive, cpuSet);
        }

        // 启动一个SubTask
        // worker: SubTask的运行代码，可以是任何可调用(_Callable)类型对象，包括function pointer、function objects、 std::function object，lambda表达式
        // priority：SubTask优先级，默认为居中的Regular
        // isExclusive: 与Launch函数同名参数意义相同，另外Exclusive Task开启的SubTask都是Exclusive，此参数可忽略；非Exclusive Task开启的SubTask是否为Exclusive由此参数决定
        // cpuSet: 限定在某些CPU上运行，默认所有CPU可用
        // args: 传给worker的可变参数列表
        // 返回: 如按预期创建thread，则在optional中被返回，否则返回空optional
        template <typename Worker, typename... Args>
        static std::optional<std::thread> SubTask(Worker worker, SubTaskPriority priority = SubTaskPriority::Regular, bool isExclusive = false, CpuSet cpuSet = UINT64_MAX,
                            Args &&...args)
        {
            // SubTask只能被Task或SubTask线程开启，而不能被Task以外线程开启
            if (workerContext.inTask != WorkerContext::InTask::MainTask && workerContext.inTask != WorkerContext::InTask::SubTask)
            {
                return std::optional<std::thread>();
            }

            return std::thread([&worker, &args...](int systemPriority, SubTaskPriority priority, bool isExclusive, CpuSet cpuSet)
            {
                // 如果限定了可用CPU，则进行相关设置
                if (!cpuSet.all())
                {
                    cpu_set_t cpu_set;
                    CPU_ZERO(&cpu_set);

                    for(std::size_t i = 0; i < cpuSet.size(); ++i)
                    {
                        if(cpuSet[i])
                        {
                            CPU_SET(i, &cpu_set);
                        }
                    }

                    if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) != 0)
                    {
                        // TODO: return erro info to caller
                        return;
                    }
                }
                // 当前线程为SubTask线程
                workerContext.inTask = WorkerContext::InTask::SubTask;

                // 根据所在Task系统定义优先级和SubTask优先级参数priority，设置Linux定义的线程优先级
                sched_param priorityParma;
                priorityParma.sched_priority = systemPriority + static_cast<int>(priority);

                // 根据isExclusive参数的说明，设置SubTask线程调度策略
                if (sched_setscheduler(0, (isExclusive || workerContext.isExclusive) ? SCHED_FIFO : SCHED_RR, &priorityParma) != 0)
                {
                    // TODO: return erro info to caller
                    return;
                }

                // 开始运行task代码
                worker(args...);
            }, workerContext.systemPriorty, priority, isExclusive, cpuSet);
        }

    private:

        // 当前线程设置和与在task的关系
        struct WorkerContext
        {
            // 当前线程与在task的关系
            enum class InTask
            {
                OutTask,    // 非task thread
                MainTask,   // Task thread
                SubTask     // SubTask thread
            };
            InTask inTask = InTask::OutTask;

            int systemPriorty;// 当前task对应的实际系统定义的线程优先级
            bool isExclusive;
        };

        //  thread local storage保存的上述状态
        thread_local static WorkerContext workerContext;
    };
}
