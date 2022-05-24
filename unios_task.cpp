#include "unios_task.h"

namespace UniOS
{
    thread_local Task::WorkerContext Task::workerContext;
}