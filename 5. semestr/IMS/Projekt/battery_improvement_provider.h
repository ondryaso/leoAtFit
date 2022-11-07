#ifndef IMS_BATTERY_IMPROVEMENT_PROVIDER_H
#define IMS_BATTERY_IMPROVEMENT_PROVIDER_H

#include <simlib.h>
#include "util.h"

class BatteryImprovement : public Event {
public:
    explicit BatteryImprovement() : Event(ADJUSTMENT_EVENTS_PRIORITY) {};

    double getCostCoefficient();

    void Behavior() override;

    void printStat();

private:
    double currentCostCoefficient = 1.0;
    Stat improvementStat;
};


#endif //IMS_BATTERY_IMPROVEMENT_PROVIDER_H
