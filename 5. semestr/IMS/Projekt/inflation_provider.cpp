#include <iostream>
#include "inflation_provider.h"
#include "sim_config.h"
#include "util.h"

double Inflation::getAccumulatedInflation() {
    return currentInflation;
}

void Inflation::Behavior() {
    double newInflation = Normal(Config.inflationMean, Config.inflationStdDev);
    inflationStat(newInflation);
    currentInflation *= 1 + newInflation;
    Activate(Time + YEAR_MT);
}

void Inflation::printStat() {
    std::cout << "---------" << std::endl;
    std::cout << "Inflation" << std::endl;
    std::cout << "---------" << std::endl;

    inflationStat.Output();

    std::cout << std::endl;
}
