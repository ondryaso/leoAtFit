#include "battery_scenario.h"
#include "sim_config.h"
#include "util.h"
#include <cmath>
#include <iostream>

BatteryScenario::BatteryScenario(Inflation *inflation, BatteryImprovement *batteryImprovement,
                                 EnergyPriceAnomaly *anomaly, BaseScenario *baseScenario)
        : Process(BATTERY_SCENARIO_PRIORITY), inflation(inflation), batteryImprovement(batteryImprovement),
          anomaly(anomaly), baseScenario(baseScenario) {

    d = Config.batteryEolRelativeCapacity * Config.R;
    // We need to store 12 hours worth of Pn - 12Pn MWh - to cover the on-peak consumption
    totalNeededEnergy = (Config.Pn * 12) / (Config.batteryEolRelativeCapacity * Config.R);
    cyclesCount = static_cast<int>(68177.0 * exp(-3.021 * d)); // excel rulez
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

void BatteryScenario::Behavior() {
    double relativeCapacity = 1.0;
    double cycleDegradationCoefficient = pow(Config.batteryEolRelativeCapacity, 1.0 / cyclesCount);
    double yearDegradationCoefficient = pow(Config.batteryEolRelativeCapacity, 1.0 / Config.Tshelf);

    // Initial battery cost
    batteryCostStat(totalNeededEnergy * Config.czkPerMwhInstallCost);

    // Energy consumed before the battery has been built
    energyCostStat(baseScenario->getTotalCost());
    energyImportedStat(baseScenario->getImportedEnergy());

    std::cout << "Battery built at time " << Time << std::endl;
    std::cout << "Energy imported before the battery was built [MWh]: " << baseScenario->getImportedEnergy() << std::endl;
    std::cout << "Cost of energy imported before the battery was built [CZK]: " << baseScenario->getTotalCost()
              << std::endl << std::endl;

    while (true) {
        // Calculate battery usage
        long hour = static_cast<long>(Time);

        if (hour % 24 == 0) {
            // Each day
            relativeCapacity *= cycleDegradationCoefficient;
        }

        if (hour % YEAR_MT == 0) {
            // Each year
            annualCostStat(totalNeededEnergy * Config.czkPerMwhAnnualMaintenance);
            relativeCapacity *= yearDegradationCoefficient;
        }

        if (relativeCapacity <= Config.batteryEolRelativeCapacity) {
            double totalNewCost = totalNeededEnergy * Config.czkPerMwhInstallCost;

            // Increase with inflation
            totalNewCost *= inflation->getAccumulatedInflation();
            // Decrease with battery technology/cost improvement
            totalNewCost *= batteryImprovement->getCostCoefficient();

            batteryCostStat(totalNewCost);
            relativeCapacity = 1;
        }

        // Calculate energy purchasing (only during off-peak)
        auto currentTime = convertModelTime();
        if (currentTime.hour < 8 || currentTime.hour > 19) {
            auto load = loadDataProvider.getLoadData(currentTime.month, currentTime.hour);
            auto neededEnergy = load * Config.Pn * 24 +
                                Config.Pn /* times 12 (total needed is 12Pn divided by 12 (there are 12 off-peak hours) */;

            auto cost =
                    neededEnergy * anomaly->getAnomalyPriceCoefficient() *
                    energyCostProvider.getCost(inflation->getAccumulatedInflation(),
                                               currentTime.month, currentTime.hour);
            energyImportedStat(neededEnergy);
            energyCostStat(cost);
        }
        Wait(1.0);
    }
}

#pragma clang diagnostic pop

void BatteryScenario::printStat() {
    std::cout << "---------------------------" << std::endl;
    std::cout << "Scenario 2: using batteries" << std::endl;
    std::cout << "---------------------------" << std::endl;

    std::cout << "Battery install cost [CZK]" << std::endl;
    std::cout << "Total: " << batteryCostStat.Sum() << " CZK" << std::endl;
    batteryCostStat.Output();

    std::cout << std::endl;
    std::cout << "Annual battery maintenance cost [CZK]" << std::endl;
    std::cout << "Total: " << annualCostStat.Sum() << " CZK" << std::endl;
    annualCostStat.Output();

    std::cout << std::endl;
    std::cout << "Imported energy [MWh]" << std::endl;
    std::cout << "Total: " << energyImportedStat.Sum() << std::endl;
    energyImportedStat.Output();

    std::cout << std::endl;
    std::cout << "Cost of imported energy [CZK]" << std::endl;
    std::cout << "Total: " << energyCostStat.Sum() << " CZK" << std::endl;
    energyCostStat.Output();

    std::cout << std::endl;
}

double BatteryScenario::getTotalCost() const {
    return batteryCostStat.Sum() + annualCostStat.Sum() + energyCostStat.Sum();
}
