#ifndef IMS_BASE_SCENARIO_H
#define IMS_BASE_SCENARIO_H

#include "simlib.h"
#include "load_data_provider.h"
#include "energy_cost_provider.h"
#include "inflation_provider.h"
#include "energy_anomaly.h"

class BaseScenario : public Process {
public:
    BaseScenario(Inflation *inflation, EnergyPriceAnomaly *anomaly)
            : Process(BASE_SCENARIO_PRIORITY), energyStat("Power"), costStat("Cost"),
              inflation(inflation), anomaly(anomaly) {}

    void printStat();

    [[nodiscard]] double getTotalCost() const;

    [[nodiscard]] double getImportedEnergy() const;

private:
    Stat energyStat;
    Stat costStat;

    Inflation *inflation;
    EnergyPriceAnomaly *anomaly;

    LoadDataProvider loadDataProvider{};
    EnergyCostProvider energyCostProvider{};

    void Behavior() override;
};


#endif //IMS_BASE_SCENARIO_H
