// self-update. --update flag
// checks github releases/latest, downloads new binary, atomically replaces
// --uninstall flag: removes ~/.local/bin/xommit and ~/.config/xommit

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

std::string user_config_dir() {
    std::string xdg = get_env("XDG_CONFIG_HOME");
    if (!xdg.empty()) {
        return xdg;
    }
    std::string home = get_env("HOME");
    if (!home.empty()) {
        return path_join(home, ".config");
    }
    return ".";
}

std::string user_bin_dir() {
    std::string xdg = get_env("XDG_BIN_HOME");
    if (!xdg.empty()) {
        return xdg;
    }
    std::string home = get_env("HOME");
    if (!home.empty()) {
        return path_join(path_join(home, ".local"), "bin");
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

} // namespace

std::string current_version() {
    return XOMMIT_VERSION;
}

std::string install_dir() {
    return user_bin_dir();
}

std::string exe_path() {
    return path_join(install_dir(), XOMMIT_ASSET);
}

void cleanup_old_binary() {
    std::error_code error;
    std::filesystem::remove(std::filesystem::path(path_join(install_dir(), "xommit.old")), error);
}

void uninstall() {
    std::string bin_path = path_join(user_bin_dir(), XOMMIT_ASSET);
    std::string config_base = path_join(user_config_dir(), "xommit");

    std::error_code error;
    std::filesystem::remove(std::filesystem::path(bin_path), error);
    std::filesystem::remove_all(std::filesystem::path(config_base), error);
    if (!error) {
        print_success("Removed all xommit data");
        print_help_hint();
    }
}

void update_binary() {
    std::string latest = latest_version();
    if (!version_less(current_version(), latest)) {
        print_success("xommit is already up to date (" + current_version() + ")");
        print_help_hint();
        return;
    }

    ensure_dir(install_dir());
    std::string temporary = path_join(install_dir(), "xommit.new");
    {
        Spinner spinner("downloading xommit " + latest);
        http_download("https://github.com/" XOMMIT_REPO "/releases/download/v" + latest + "/" XOMMIT_ASSET,
                      temporary);
    }

    std::error_code error;
    auto size = std::filesystem::file_size(std::filesystem::path(temporary), error);
    if (error || size < 100000) {
        std::filesystem::remove(std::filesystem::path(temporary), error);
        throw std::runtime_error("downloaded file looks invalid");
    }

    std::string old = path_join(install_dir(), "xommit.old");
    std::filesystem::remove(std::filesystem::path(old), error);

    if (file_exists(exe_path())) {
        std::filesystem::rename(std::filesystem::path(exe_path()),
                                std::filesystem::path(old), error);
        if (error) {
            std::filesystem::remove(std::filesystem::path(temporary), error);
            throw std::runtime_error("cannot replace the running binary");
        }
    }

    std::filesystem::rename(std::filesystem::path(temporary),
                            std::filesystem::path(exe_path()), error);
    if (error) {
        std::filesystem::rename(std::filesystem::path(old),
                                std::filesystem::path(exe_path()), error);
        throw std::runtime_error("cannot install the new binary");
    }

    // Make executable
    std::filesystem::permissions(exe_path(),
        std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read | std::filesystem::perms::owner_write |
        std::filesystem::perms::group_exec | std::filesystem::perms::group_read |
        std::filesystem::perms::others_exec | std::filesystem::perms::others_read, error);

    print_success("updated to xommit " + latest);
    print_help_hint();
}

} // namespace xommit