#pragma once

#include <string>
#include <vector>

namespace xommit {

const char* system_prompt();
std::string build_user_prompt(const std::string& hint, const std::string& diff_context);
std::vector<std::string> parse_commit_options(const std::string& response);
std::string sanitize_message(const std::string& value);

}
