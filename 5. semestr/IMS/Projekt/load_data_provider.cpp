#include <stdexcept>
#include "load_data_provider.h"
#include "sim_config.h"

using std::string;


double LoadDataProvider::getLoadData(unsigned int month, unsigned int hour) const {
    if (month > 11 || hour > 23) {
        throw std::runtime_error("Invalid month or hour.");
    }

    auto val = values[month][hour];
    return Config.includePsh ? val.withPsh : val.withoutPsh;
}