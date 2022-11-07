#include <simlib.h>
#include "load_data_provider.h"
#include "energy_cost_provider.h"
#include "inflation_provider.h"
#include "battery_improvement_provider.h"
#include "energy_anomaly.h"
#include "base_scenario.h"

#ifndef IMS_BATTERY_SCENARIO_H
#define IMS_BATTERY_SCENARIO_H

class BatteryScenario : public Process {
public:
    explicit BatteryScenario(Inflation *inflation, BatteryImprovement *batteryImprovement,
                             EnergyPriceAnomaly *anomaly, BaseScenario *baseScenario);

    void Behavior() override;

    void printStat();

    [[nodiscard]] double getTotalCost() const;

private:
    double d; // Depth of discharge
    double totalNeededEnergy;
    int cyclesCount;

    Stat batteryCostStat;
    Stat annualCostStat;
    Stat energyCostStat;
    Stat energyImportedStat;

    LoadDataProvider loadDataProvider{};
    EnergyCostProvider energyCostProvider{};

    Inflation *inflation;
    BatteryImprovement *batteryImprovement;
    EnergyPriceAnomaly *anomaly;
    BaseScenario *baseScenario;
};


#endif //IMS_BATTERY_SCENARIO_H
