#include "battery_improvement_provider.h"

#include <iostream>
#include "sim_config.h"
#include "util.h"

double BatteryImprovement::getCostCoefficient() {
    return currentCostCoefficient;
}

void BatteryImprovement::Behavior() {
    double yearImprovement = Uniform(Config.batteryCostImprovementMin, Config.batteryCostImprovementMax);

    improvementStat(yearImprovement);
    currentCostCoefficient *= 1 - yearImprovement;
    Activate(Time + YEAR_MT);
}

void BatteryImprovement::printStat() {
    std::cout << "--------------------" << std::endl;
    std::cout << "Battery cost decline" << std::endl;
    std::cout << "--------------------" << std::endl;

    improvementStat.Output();

    std::cout << std::endl;
}
