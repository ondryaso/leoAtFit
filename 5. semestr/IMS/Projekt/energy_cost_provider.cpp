#include <stdexcept>
#include "energy_cost_provider.h"

double EnergyCostProvider::getCost(double inflationAccumulated, unsigned int month, unsigned int hour) const {
    if (month > 11) {
        throw std::runtime_error("Not a valid month number");
    }

    if (hour < 8 || hour > 19) {
        // offpeak
        return values[month].offpeak * inflationAccumulated;
    } else {
        // onpeak
        return values[month].onpeak * inflationAccumulated;
    }
}
