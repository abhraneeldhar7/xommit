// self-update logic
// current_version: returns XOMMIT_VERSION
// install_dir: %LOCALAPPDATA%\xommit\bin
// exe_path: install_dir + xommit.exe
// update_binary: --update flag, downloads latest from github releases
// uninstall: --uninstall flag, removes %LOCALAPPDATA%\xommit and %APPDATA%\xommit

#pragma once

#include <string>

namespace xommit {

std::string current_version();
std::string install_dir();
std::string exe_path();
void cleanup_old_binary();
void update_binary();
void uninstall();

}
