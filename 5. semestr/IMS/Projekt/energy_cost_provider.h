#ifndef IMS_ENERGY_COST_PROVIDER_H
#define IMS_ENERGY_COST_PROVIDER_H

class EnergyCostProvider {
public:
    [[nodiscard]] double getCost(double inflationAccumulated, unsigned int month, unsigned int hour) const;

private:
    // Taken from OTE-CR, considered as baseline price for 2020
    struct AverageMonthlyEnergyCost {
        double offpeak; // price in CZK/MWh off peak (0-8, 20-24)
        double onpeak; // price in CZK/MWh on peak (8 - 20)
    } values[12] = {{853.1614, 1302.0503},
                    {799.8559, 1097.4667},
                    {722.8245, 881.4854},
                    {757.6083, 773.1429},
                    {736.6408, 794.9556},
                    {824.0667, 951.06075},
                    {938.5847, 1084.3661},
                    {920.0719, 1057.8088},
                    {967.3081, 1156.9835},
                    {912.2259, 1238.2405},
                    {924.8861, 1332.8470},
                    {825.4965, 1283.1795}};

};


#endif //IMS_ENERGY_COST_PROVIDER_H