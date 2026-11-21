#pragma once

#include <string>

namespace xommit {

struct ModelSpec {
    std::string provider;
    std::string id;
    std::string name;
    int context_window = 0;
    int max_output_tokens = 0;
    bool free = false;
    bool recommended = false;
    double input_price = 0.0;
    double output_price = 0.0;
};

}
