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
        throw std::runtime_error("no saved credentials, run xommit --connect");
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
    const Json* model = root.find("model");
    if (config.provider.empty() || config.api_key.empty() || model == nullptr || !model->is_object()) {
        throw std::runtime_error("config file is incomplete, run xommit --connect");
    }

    config.model.provider = config.provider;
    config.model.id = model->get_string("id");
    config.model.name = model->get_string("name");
    config.model.context_window = static_cast<int>(model->get_number("context_window"));
    config.model.max_output_tokens = static_cast<int>(model->get_number("max_output_tokens"));
    config.model.free = model->get_bool("free");
    if (config.model.id.empty()) {
        throw std::runtime_error("config file has no model, run xommit --connect");
    }
    return config;
}

void save_config(const Config& config) {
    Json model = json_object({
        {"id", Json(config.model.id)},
        {"name", Json(config.model.name)},
        {"context_window", Json(config.model.context_window)},
        {"max_output_tokens", Json(config.model.max_output_tokens)},
        {"free", Json(config.model.free)},
    });

    Json root = json_object({
        {"provider", Json(config.provider)},
        {"api_key", Json(config.api_key)},
        {"model", model},
    });

    ensure_dir(path_join(appdata_dir(), "xommit"));
    write_file(config_path(), root.dump() + "\n");
}

void reset_config() {
    std::error_code error;
    std::filesystem::remove(std::filesystem::path(utf8_to_wide(config_path())), error);
}

}
