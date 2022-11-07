#include <stdexcept>
#include "util.h"
#include "simlib.h"

DateTime convertModelTime() {
    long time = static_cast<long>(Time);
    // Start of simulation is 1st January 2021 (data ends there)
    DateTime result{};
    result.year = time / YEAR_MT;
    long remainingHours = time % YEAR_MT;
    unsigned monthLength[] = {31 * 24, 28 * 24, 31 * 24, 30 * 24, 31 * 24 , 30 * 24,
                              31 * 24, 31 * 24, 30 * 24, 31 * 24, 30 * 24, 31 * 24};

    for (int i = 0; i < 12; i++) {
        if (remainingHours <= monthLength[i]) {
            result.month = i;
            result.hour = remainingHours % 24;
            return result;
        }

        remainingHours -= monthLength[i];
    }

    throw std::runtime_error("Error in time calculation (this should not happen).");
}