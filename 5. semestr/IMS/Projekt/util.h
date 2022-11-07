#ifndef IMS_UTIL_H
#define IMS_UTIL_H

#define YEAR_MT 8760 // 365 * 24
#define MONTH_MT 730 // 365 * 24 / 12

#define ADJUSTMENT_EVENTS_PRIORITY 10
#define BASE_SCENARIO_PRIORITY 5
#define BATTERY_SCENARIO_PRIORITY 1

struct DateTime {
    unsigned int hour;
    unsigned int month;
    unsigned int year;
};

DateTime convertModelTime();

#endif //IMS_UTIL_H
