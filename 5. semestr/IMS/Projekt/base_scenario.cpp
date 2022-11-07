#include <iostream>
#include "base_scenario.h"
#include "util.h"
#include "sim_config.h"


void BaseScenario::Behavior() {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        auto currentTime = convertModelTime();
        auto load = loadDataProvider.getLoadData(currentTime.month, currentTime.hour);
        // The power of Dukovany is Pn MW -> daily production is 24Pn MWh
        // In this hour we would use (load * 100)%
        auto neededEnergy = load * Config.Pn * 24;

        auto cost = neededEnergy * anomaly->getAnomalyPriceCoefficient() *
                    energyCostProvider.getCost(inflation->getAccumulatedInflation(),
                                               currentTime.month, currentTime.hour);

        energyStat(neededEnergy);
        costStat(cost);

        Wait(1);
    }
#pragma clang diagnostic pop
}


void BaseScenario::printStat() {
    std::cout << "--------------------------" << std::endl;
    std::cout << "Scenario 1: only importing" << std::endl;
    std::cout << "--------------------------" << std::endl;

    std::cout << "Imported energy [MWh]" << std::endl;
    std::cout << "Total: " << energyStat.Sum() << std::endl;
    energyStat.Output();

    std::cout << std::endl;
    std::cout << "Cost of imported energy [CZK]" << std::endl;
    std::cout << "Total: " << costStat.Sum() << std::endl;
    costStat.Output();

    std::cout << std::endl;
}

[[nodiscard]] double BaseScenario::getTotalCost() const {
    return costStat.Sum();
}

[[nodiscard]] double BaseScenario::getImportedEnergy() const {
    return energyStat.Sum();
}
