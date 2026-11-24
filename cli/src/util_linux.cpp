// utils. string: trim, to_lower, starts_with, ends_with, contains, replace_all, split, join
// env: get_env
// encoding: utf8 native on linux
// files: file_exists, read_file, write_file, ensure_dir, path_join

#include "xommit/util.h"

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace xommit {

std::string trim(const std::string& value) {
    size_t start = 0;
    size_t end = value.size();
    while (start < end && std::isspace(static_cast<unsigned char>(value[start]))) {
        start++;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        end--;
    }
    return value.substr(start, end - start);
}

std::string to_lower(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

bool starts_with(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool ends_with(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool contains(const std::string& value, const std::string& needle) {
    return value.find(needle) != std::string::npos;
}

std::string replace_all(std::string value, const std::string& from, const std::string& to) {
    if (from.empty()) {
        return value;
    }
    size_t position = 0;
    while ((position = value.find(from, position)) != std::string::npos) {
        value.replace(position, from.size(), to);
        position += to.size();
    }
    return value;
}

std::vector<std::string> split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : value) {
        if (c == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
    return parts;
}

std::string join(const std::vector<std::string>& values, const std::string& separator) {
    std::string result;
    for (size_t i = 0; i < values.size(); i++) {
        if (i > 0) {
            result += separator;
        }
        result += values[i];
    }
    return result;
}

std::string get_env(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
}

std::wstring utf8_to_wide(const std::string& value) {
    if (value.empty()) {
        return L"";
    }
    std::mbstate_t state{};
    const char* src = value.c_str();
    size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (len == static_cast<size_t>(-1)) {
        return L"";
    }
    std::wstring result(len, L'\0');
    state = std::mbstate_t{};
    src = value.c_str();
    std::mbsrtowcs(result.data(), &src, len, &state);
    return result;
}

std::string wide_to_utf8(const std::wstring& value) {
    if (value.empty()) {
        return "";
    }
    std::mbstate_t state{};
    const wchar_t* src = value.c_str();
    size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (len == static_cast<size_t>(-1)) {
        return "";
    }
    std::string result(len, '\0');
    state = std::mbstate_t{};
    src = value.c_str();
    std::wcsrtombs(result.data(), &src, len, &state);
    return result;
}

std::string windows_error(unsigned long code) {
    // Not used on Linux
    return "error code: " + std::to_string(code);
}

bool file_exists(const std::string& path) {
    std::error_code error;
    return std::filesystem::exists(std::filesystem::path(path), error);
}

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

void write_file(const std::string& path, const std::string& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot write " + path);
    }
    file.write(data.data(), static_cast<std::streamsize>(data.size()));
}

void ensure_dir(const std::string& path) {
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(path), error);
    if (error) {
        throw std::runtime_error("cannot create " + path);
    }
}

std::string path_join(const std::string& left, const std::string& right) {
    if (left.empty()) {
        return right;
    }
    if (right.empty()) {
        return left;
    }
    char last = left.back();
    if (last == '/') {
        return left + right;
    }
    return left + "/" + right;
}

} // namespace xommit