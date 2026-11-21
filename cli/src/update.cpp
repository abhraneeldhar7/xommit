#include "xommit/update.h"

#include <filesystem>
#include <stdexcept>

#include "xommit/http.h"
#include "xommit/json.h"
#include "xommit/spinner.h"
#include "xommit/ui.h"
#include "xommit/util.h"
#include "xommit/version.h"

namespace xommit {

namespace {

std::string local_appdata() {
    std::string value = get_env("LOCALAPPDATA");
    if (!value.empty()) {
        return value;
    }
    std::string profile = get_env("USERPROFILE");
    if (!profile.empty()) {
        return path_join(profile, "AppData\\Local");
    }
    return ".";
}

bool version_less(const std::string& left, const std::string& right) {
    std::vector<std::string> a = split(left, '.');
    std::vector<std::string> b = split(right, '.');
    size_t count = std::max(a.size(), b.size());
    for (size_t i = 0; i < count; i++) {
        int value_a = i < a.size() ? std::atoi(a[i].c_str()) : 0;
        int value_b = i < b.size() ? std::atoi(b[i].c_str()) : 0;
        if (value_a != value_b) {
            return value_a < value_b;
        }
    }
    return false;
}

std::string latest_version() {
    HttpResponse response = http_request(
        "GET",
        "https://api.github.com/repos/" XOMMIT_REPO "/releases/latest",
        {"User-Agent: xommit/" XOMMIT_VERSION, "Accept: application/vnd.github+json"},
        "");
    if (response.status != 200) {
        throw std::runtime_error("cannot check latest release (HTTP " + std::to_string(response.status) + ")");
    }
    Json root = Json::parse(response.body);
    std::string tag = root.get_string("tag_name");
    if (starts_with(tag, "v")) {
        tag = tag.substr(1);
    }
    if (tag.empty()) {
        throw std::runtime_error("latest release has no version tag");
    }
    return tag;
}

}

std::string current_version() {
    return XOMMIT_VERSION;
}

std::string install_dir() {
    return path_join(path_join(local_appdata(), "xommit"), "bin");
}

std::string exe_path() {
    return path_join(install_dir(), XOMMIT_ASSET);
}

std::string version_file_path() {
    return path_join(install_dir(), "version.txt");
}

void cleanup_old_binary() {
    std::error_code error;
    std::filesystem::remove(std::filesystem::path(utf8_to_wide(path_join(install_dir(), "xommit.old.exe"))), error);
}

void update_binary() {
    std::string latest = latest_version();
    if (!version_less(current_version(), latest)) {
        print_success("xommit is already up to date (" + current_version() + ")");
        return;
    }

    ensure_dir(install_dir());
    std::string temporary = path_join(install_dir(), "xommit.new.exe");
    {
        Spinner spinner("downloading xommit " + latest);
        http_download("https://github.com/" XOMMIT_REPO "/releases/download/v" + latest + "/" XOMMIT_ASSET,
                      temporary);
    }

    std::error_code error;
    auto size = std::filesystem::file_size(std::filesystem::path(utf8_to_wide(temporary)), error);
    if (error || size < 100000) {
        std::filesystem::remove(std::filesystem::path(utf8_to_wide(temporary)), error);
        throw std::runtime_error("downloaded file looks invalid");
    }

    std::string old = path_join(install_dir(), "xommit.old.exe");
    std::filesystem::remove(std::filesystem::path(utf8_to_wide(old)), error);

    if (file_exists(exe_path())) {
        std::filesystem::rename(std::filesystem::path(utf8_to_wide(exe_path())),
                                std::filesystem::path(utf8_to_wide(old)), error);
        if (error) {
            std::filesystem::remove(std::filesystem::path(utf8_to_wide(temporary)), error);
            throw std::runtime_error("cannot replace the running binary");
        }
    }

    std::filesystem::rename(std::filesystem::path(utf8_to_wide(temporary)),
                            std::filesystem::path(utf8_to_wide(exe_path())), error);
    if (error) {
        std::filesystem::rename(std::filesystem::path(utf8_to_wide(old)),
                                std::filesystem::path(utf8_to_wide(exe_path())), error);
        throw std::runtime_error("cannot install the new binary");
    }

    write_file(version_file_path(), latest + "\n");
    print_success("updated to xommit " + latest);
}

}
