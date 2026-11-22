// prompt building + response parsing
// system_prompt: tells ai to write conventional commits
// build_user_prompt: combines hint + diff context
// parse_commit_options: extracts 3 messages from ai response
// sanitize_message: validates conventional commit format

#pragma once

#include <string>
#include <vector>

namespace xommit {

const char* system_prompt();
std::string build_user_prompt(const std::string& hint, const std::string& diff_context);
std::vector<std::string> parse_commit_options(const std::string& response);
std::string sanitize_message(const std::string& value);

}
