#pragma once

#include <string>
#include <vector>

namespace xommit {

std::string trim(const std::string& value);
std::string to_lower(std::string value);
bool starts_with(const std::string& value, const std::string& prefix);
bool ends_with(const std::string& value, const std::string& suffix);
bool contains(const std::string& value, const std::string& needle);
std::string replace_all(std::string value, const std::string& from, const std::string& to);
std::vector<std::string> split(const std::string& value, char delimiter);
std::string join(const std::vector<std::string>& values, const std::string& separator);

std::string get_env(const std::string& name);
std::wstring utf8_to_wide(const std::string& value);
std::string wide_to_utf8(const std::wstring& value);
std::string windows_error(unsigned long code);

bool file_exists(const std::string& path);
std::string read_file(const std::string& path);
void write_file(const std::string& path, const std::string& data);
void ensure_dir(const std::string& path);
std::string path_join(const std::string& left, const std::string& right);

}
