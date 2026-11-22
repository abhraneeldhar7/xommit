// utils. trim, split, join, starts_with, get_env
// utf8/wide conversion, path join, file read/write
// stdout_is_tty, set_utf8, enable_vt

#include "xommit/util.h"

#include <windows.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <stdexcept>

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
    std::wstring wide_name = utf8_to_wide(name);
    DWORD size = GetEnvironmentVariableW(wide_name.c_str(), nullptr, 0);
    if (size == 0) {
        return "";
    }
    std::wstring value(size, L'\0');
    DWORD written = GetEnvironmentVariableW(wide_name.c_str(), value.data(), size);
    if (written == 0) {
        return "";
    }
    value.resize(written);
    return wide_to_utf8(value);
}

std::wstring utf8_to_wide(const std::string& value) {
    if (value.empty()) {
        return L"";
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (size <= 0) {
        return L"";
    }
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::string wide_to_utf8(const std::wstring& value) {
    if (value.empty()) {
        return "";
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                                   nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return "";
    }
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                        result.data(), size, nullptr, nullptr);
    return result;
}

std::string windows_error(unsigned long code) {
    wchar_t* buffer = nullptr;
    DWORD size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_IGNORE_INSERTS,
                                nullptr, code, 0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
    std::wstring message = size > 0 && buffer != nullptr ? std::wstring(buffer, size) : L"unknown error";
    if (buffer != nullptr) {
        LocalFree(buffer);
    }
    return trim(wide_to_utf8(message));
}

bool file_exists(const std::string& path) {
    std::error_code error;
    return std::filesystem::exists(std::filesystem::path(utf8_to_wide(path)), error);
}

std::string read_file(const std::string& path) {
    std::ifstream file(std::filesystem::path(utf8_to_wide(path)), std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

void write_file(const std::string& path, const std::string& data) {
    std::ofstream file(std::filesystem::path(utf8_to_wide(path)), std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot write " + path);
    }
    file.write(data.data(), static_cast<std::streamsize>(data.size()));
}

void ensure_dir(const std::string& path) {
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::path(utf8_to_wide(path)), error);
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
    if (last == '\\' || last == '/') {
        return left + right;
    }
    return left + "\\" + right;
}

}
