#ifndef IMS_SIM_CONFIG_H
#define IMS_SIM_CONFIG_H

struct SimConfig {
    double Pn = 500; // Power needed [MW]
    double R = 0.5; // Extra capacity coefficient [-]

    int scenarioStartMonth = 0; // Starting time of scenario simulation [months]
    int totalYears = 15; // Maximum time of simulation [years]

    double Tshelf = 13; // The "lifetime" of a battery - represents the battery's implicit capacity decay [years]
    double batteryEolRelativeCapacity = 0.8; // The percentage of capacity of a battery that is considered its "end-of-life" value [ratio]
    // Taken from https://www.tesla.com/megapack/design
    double czkPerMwhInstallCost = 7200000; // Price of new battery installation/overhaul [CZK/MWh]
    double czkPerMwhAnnualMaintenance = 28000; // Annual maintenance cost of the battery [CZK/MWh]

    // Year decline in battery cost [%]
    double batteryCostImprovementMin = 0.13;
    double batteryCostImprovementMax = 0.17;

    // Inflation rate [%]
    double inflationMean = 0.03; // Inflation rate mean value (p.a.)
    double inflationStdDev = 0.005; // Inflation rate standard deviation

    // Energy cost anomalies
    double energyCostAnomalyRateMean = 9; // Anomaly rate [months]
    double energyCostAnomalyMin = 0.2; // Energy cost peak increase minimum [ratio]
    double energyCostAnomalyMax = 1.0; // Energy cost peak increase maximum [ratio]

    double batteryBuildTime = 6; // Time it takes to build the battery [months]

    // Whether the load of pumped-storage hydroelectricity facilities should be considered in the load distribution
    bool includePsh = false;
};

extern SimConfig Config;

#endif //IMS_SIM_CONFIG_H
