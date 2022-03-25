#include "func.h"
#include "logging.hpp"

void func()
{
    LOGGER(logging::error) << "in func";
}