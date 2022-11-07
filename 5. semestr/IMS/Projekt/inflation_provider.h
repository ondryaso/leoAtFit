#ifndef IMS_INFLATION_PROVIDER_H
#define IMS_INFLATION_PROVIDER_H

#include <simlib.h>
#include "util.h"

class Inflation : public Event {
public:
    explicit Inflation() : Event(ADJUSTMENT_EVENTS_PRIORITY) {}

    double getAccumulatedInflation();

    void Behavior() override;

    void printStat();

private:
    double currentInflation = 1.0;
    Stat inflationStat;
};

#endif //IMS_INFLATION_PROVIDER_H
