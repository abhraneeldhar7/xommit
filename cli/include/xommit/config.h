#pragma once

#include <string>

#include "xommit/model.h"

namespace xommit {

struct Config {
    std::string provider;
    std::string api_key;
    std::string model_id;
};

std::string config_path();
bool config_exists();
Config load_config();
void save_config(const Config& config);
void reset_config();

}
