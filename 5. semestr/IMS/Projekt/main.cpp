#include <iostream>
#include <simlib.h>
#include <cstring>
#include "sim_config.h"
#include "base_scenario.h"
#include "battery_scenario.h"
#include "util.h"
#include "energy_anomaly.h"

// A global configuration object used throughout the program
SimConfig Config;

int main(int argc, char **argv) {
    if (argc > 1) {
        if (strcmp(argv[1], "--randomize") == 0) {
            RandomSeed(time(nullptr));
        } else if (argc > 2 && strcmp(argv[1], "--seed") == 0) {
            long seed = std::strtol(argv[2], nullptr, 10);
            RandomSeed(seed);
        } else {
            std::cout
                    << "You can use either --randomize to use a current time based seed, or --seed [value] to provide a specific seed."
                    << std::endl;
            return 1;
        }
    }


    Init(0, Config.totalYears * YEAR_MT);

    auto *inflation = new Inflation();
    inflation->Activate(YEAR_MT);

    auto *batteryCostImprovement = new BatteryImprovement();
    batteryCostImprovement->Activate(YEAR_MT);

    auto *anomaly = new EnergyPriceAnomaly();
    anomaly->Activate(Exponential(Config.energyCostAnomalyRateMean * MONTH_MT));

    auto *baseScenario = new BaseScenario(inflation, anomaly);
    baseScenario->Activate(Config.scenarioStartMonth * MONTH_MT);

    auto *batteryScenario = new BatteryScenario(inflation, batteryCostImprovement, anomaly, baseScenario);
    batteryScenario->Activate((Config.scenarioStartMonth + Config.batteryBuildTime) * MONTH_MT);

    Run();

    baseScenario->printStat();
    batteryScenario->printStat();
    inflation->printStat();
    batteryCostImprovement->printStat();
    anomaly->printStat();

    if (batteryScenario->getTotalCost() < baseScenario->getTotalCost()) {
        std::cout << "------ BATTERY SCENARIO WAS MORE COST-EFFECTIVE ("
                  << (batteryScenario->getTotalCost() * 100.0 / baseScenario->getTotalCost())
                  << "% of base scenario price). ------" << std::endl;
    } else {
        std::cout << "------ Only importing energy was more cost-effective (battery scenario is "
                  << (batteryScenario->getTotalCost() * 100.0 / baseScenario->getTotalCost() - 100.0)
                  << "% worse). ------" << std::endl;
    }

    std::cout << std::endl;
    SIMLIB_statistics.Output();
    return 0;
}
