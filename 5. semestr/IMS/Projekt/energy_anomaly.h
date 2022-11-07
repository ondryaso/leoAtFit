#ifndef IMS_ENERGY_ANOMALY_H
#define IMS_ENERGY_ANOMALY_H

#include <simlib.h>
#include "util.h"

class EnergyPriceAnomalyDecay;

class EnergyPriceAnomaly : public Event {
public:
    explicit EnergyPriceAnomaly() : Event(ADJUSTMENT_EVENTS_PRIORITY) {}

    double getAnomalyPriceCoefficient();

    void Behavior() override;

    void printStat();

private:
    double currentCoefficient = 1.0;

    Stat anomalyStat{};

    friend EnergyPriceAnomalyDecay;
};

class EnergyPriceAnomalyDecay : public Process {
public:
    explicit EnergyPriceAnomalyDecay(EnergyPriceAnomaly *anomaly);

    void Behavior() override;

private:
    EnergyPriceAnomaly *anomaly;
};

#endif //IMS_ENERGY_ANOMALY_H
