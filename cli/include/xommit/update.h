#pragma once

#include <string>

namespace xommit {

std::string current_version();
std::string install_dir();
std::string exe_path();
std::string version_file_path();
void cleanup_old_binary();
void update_binary();

}
