#include <simlib.h>
#include <iostream>
#include "energy_anomaly.h"
#include "sim_config.h"

double EnergyPriceAnomaly::getAnomalyPriceCoefficient() {
    return currentCoefficient;
}

void EnergyPriceAnomaly::Behavior() {
    currentCoefficient = 1.0 + Uniform(Config.energyCostAnomalyMin, Config.energyCostAnomalyMax);
    anomalyStat(currentCoefficient);
    new EnergyPriceAnomalyDecay(this);
}

void EnergyPriceAnomaly::printStat() {
    std::cout << "----------------------" << std::endl;
    std::cout << "Energy price anomalies" << std::endl;
    std::cout << "----------------------" << std::endl;

    anomalyStat.Output();

    std::cout << std::endl;
}

EnergyPriceAnomalyDecay::EnergyPriceAnomalyDecay(EnergyPriceAnomaly *anomaly) : anomaly(anomaly) {
    Activate();
}

void EnergyPriceAnomalyDecay::Behavior() {
    while (anomaly->currentCoefficient > 1) {
        anomaly->currentCoefficient -= 0.1f;
        Wait(MONTH_MT);
    }

    anomaly->currentCoefficient = 1.0;
    anomaly->Activate(Time + Exponential(Config.energyCostAnomalyRateMean * MONTH_MT));
}
