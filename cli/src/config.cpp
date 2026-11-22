// config file handling. reads/writes %APPDATA%\xommit\config.json
// provider, api_key, model_id

#include "xommit/config.h"

#include <filesystem>
#include <stdexcept>

#include "xommit/json.h"
#include "xommit/util.h"

namespace xommit {

namespace {

std::string appdata_dir() {
    std::string appdata = get_env("APPDATA");
    if (!appdata.empty()) {
        return appdata;
    }
    std::string profile = get_env("USERPROFILE");
    if (!profile.empty()) {
        return path_join(profile, "AppData\\Roaming");
    }
    return ".";
}

}

std::string config_path() {
    return path_join(path_join(appdata_dir(), "xommit"), "config.json");
}

bool config_exists() {
    return file_exists(config_path());
}

Config load_config() {
    std::string text = read_file(config_path());
    if (text.empty()) {
        throw std::runtime_error("no saved credentials");
    }

    Json root;
    try {
        root = Json::parse(text);
    } catch (const std::exception&) {
        throw std::runtime_error("config file is corrupt, run xommit --reset");
    }

    Config config;
    config.provider = root.get_string("provider");
    config.api_key = root.get_string("api_key");
    config.model_id = root.get_string("model_id");
    if (config.provider.empty() || config.api_key.empty() || config.model_id.empty()) {
        throw std::runtime_error("config file is incomplete, run xommit --reset");
    }
    return config;
}

void save_config(const Config& config) {
    Json root = json_object({
        {"provider", Json(config.provider)},
        {"api_key", Json(config.api_key)},
        {"model_id", Json(config.model_id)},
    });

    ensure_dir(path_join(appdata_dir(), "xommit"));
    write_file(config_path(), root.dump() + "\n");
}

void reset_config() {
    std::error_code error;
    std::filesystem::remove(std::filesystem::path(utf8_to_wide(config_path())), error);
}

}
