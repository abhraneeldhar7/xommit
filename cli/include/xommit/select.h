// interactive picker. select_option(options, title) -> index or -1
// arrow keys, type-to-filter, pgup/pgdn, home/end, esc=cancel
// 10-row viewport, non-tty falls back to numbered list

#pragma once

#include <string>
#include <vector>

namespace xommit {

int select_option(const std::vector<std::string>& options, const std::string& title);

}
