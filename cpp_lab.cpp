#include "logging.hpp"
#include "func.h"

int main(int, char *[])
{
    logging::init(logging::trace, "debug.log");
    func();
    LOGGER(logging::trace) << "track";
    LOGGER(logging::debug) << "Keep";
    LOGGER(logging::info) << "It";
    LOGGER(logging::warning) << "Simple";
    LOGGER(logging::error) << "Stupid";
    LOGGER(logging::fatal) << "Dead";

    return 0;
}