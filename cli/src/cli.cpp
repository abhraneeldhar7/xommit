#include "xommit/cli.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "xommit/config.h"
#include "xommit/diff.h"
#include "xommit/git.h"
#include "xommit/ignore.h"
#include "xommit/prompt.h"
#include "xommit/provider.h"
#include "xommit/select.h"
#include "xommit/spinner.h"
#include "xommit/ui.h"
#include "xommit/update.h"
#include "xommit/util.h"
#include "xommit/version.h"

namespace xommit {

namespace {

// Forward declarations
int run_connect();

// Centralized color/style constants
const char* C_RESET    = "\033[0m";
const char* C_BOLD     = "\033[1m";
const char* C_DIM      = "\033[90m";
const char* C_GREEN    = "\033[1;32m";
const char* C_CYAN     = "\033[1;36m";

inline std::string style_bold(const std::string& s) { return stdout_is_tty() ? (C_BOLD + s + C_RESET) : s; }
inline std::string style_dim(const std::string& s)  { return stdout_is_tty() ? (C_DIM  + s + C_RESET) : s; }
inline std::string style_green(const std::string& s){ return stdout_is_tty() ? (C_GREEN + s + C_RESET) : s; }
inline std::string style_cyan(const std::string& s) { return stdout_is_tty() ? (C_CYAN  + s + C_RESET) : s; }

// Links
const char* LINK_XOMMIT = "https://xommit.antk.in";

void print_help() {
    std::cout << "\n";
    print_logo();
    std::cout << "\n";

    std::cout << style_dim("Hello, I made ") << style_cyan("xommit") << style_dim(" as a cli tool to turn your vague commits into conventional like a pro :D") << "\n";
    std::cout << style_dim("  ") << style_cyan(LINK_XOMMIT) << "\n";
    std::cout << "\n";

    std::cout << "version " << style_bold(XOMMIT_VERSION) << "\n";
    std::cout << "\n";

    std::cout << style_dim("xommit") << " " << style_green("<your message (optional)>") << "      generate commit messages from staged changes\n";
    std::cout << style_dim("xommit") << " " << style_green("--connect") << "              save a provider, API key, and model\n";
    std::cout << style_dim("xommit") << " " << style_green("--reset") << "                delete the saved config\n";
    std::cout << style_dim("xommit") << " " << style_green("--update") << "               download the latest release\n";
    std::cout << style_dim("xommit") << " " << style_green("--version") << "              print the version\n";
    std::cout << style_dim("xommit") << " " << style_green("--help") << "                 print this help\n";
    std::cout << "\n";
}

std::vector<std::string> to_args(int argc, wchar_t** argv) {
    std::vector<std::string> result;
    for (int i = 1; i < argc; i++) {
        result.push_back(wide_to_utf8(argv[i]));
    }
    return result;
}

// Resolve model_id to full ModelSpec by fetching model list
ModelSpec resolve_model(const std::string& provider_id, const std::string& api_key, const std::string& model_id) {
    std::unique_ptr<Provider> provider = make_provider(provider_id);
    std::vector<ModelSpec> models = provider->list_models(api_key);
    for (const ModelSpec& m : models) {
        if (m.id == model_id) {
            return m;
        }
    }
    throw std::runtime_error("model not found: " + model_id);
}

template <typename Function>
auto with_retry(const std::string& label, Function function) -> decltype(function()) {
    for (int attempt = 0; attempt < 2; attempt++) {
        try {
            Spinner spinner(label);
            return function();
        } catch (const ProviderError& error) {
            if (attempt == 0 && error.retryable()) {
                continue;
            }
            throw;
        } catch (const std::exception&) {
            if (attempt == 0) {
                continue;
            }
            throw;
        }
    }
    throw std::runtime_error("request failed");
}

void print_provider_model(const Config& config) {
    std::cout << style_dim("provider: ") << config.provider << "  " << style_dim("model: ") << config.model_id << "\n\n";
}

int run_commit(const std::string& hint) {
    std::cout << "\n";
    print_logo();
    std::cout << "\n";

    std::string root = git_repo_root();
    std::string raw = git_diff_cached();
    if (trim(raw).empty()) {
        print_dim("No files are staged");
        return 0;
    }

    std::vector<DiffFile> files = parse_diff(raw);
    IgnoreRules rules(root);
    std::vector<DiffFile> sendable;
    for (const DiffFile& file : files) {
        if (!rules.is_ignored(file.path)) {
            sendable.push_back(file);
        }
    }
    if (sendable.empty()) {
        print_dim("only ignored files are staged, nothing to send to the AI");
        return 0;
    }

    // Load config and resolve model
    Config config;
    try {
        config = load_config();
    } catch (const std::exception&) {
        std::cout << style_dim("Connect your AI provider to begin") << "\n\n";
        return run_connect();
    }

    ModelSpec model;
    try {
        model = resolve_model(config.provider, config.api_key, config.model_id);
    } catch (const std::exception& e) {
        print_error("failed to resolve model: " + std::string(e.what()));
        return 1;
    }

    print_provider_model(config);

    int budget = context_budget_chars(model, system_prompt(), hint);
    bool truncated = false;
    std::string context = build_diff_context(sendable, budget, truncated);
    std::string user = build_user_prompt(hint, context);

    std::unique_ptr<Provider> provider = make_provider(config.provider);
    std::string response;
    try {
        response = with_retry("asking " + model.id, [&]() {
            return provider->complete(config.api_key, model, system_prompt(), user);
        });
    } catch (const std::exception& e) {
        std::string msg = e.what();
        if (msg.find("network") != std::string::npos ||
            msg.find("connection") != std::string::npos ||
            msg.find("timeout") != std::string::npos ||
            msg.find("resolve") != std::string::npos) {
            print_error("Can't reach AI models");
        } else {
            print_error(msg);
        }
        return 1;
    }

    std::vector<std::string> options = parse_commit_options(response);
    if (options.empty()) {
        print_error("could not read commit messages from the model response");
        return 1;
    }

    print_provider_model(config);

    int choice = select_option(options, "Choose a commit message:");
    if (choice < 0) {
        print_dim("cancelled");
        return 1;
    }
    return git_commit(options[choice]);
}

int run_connect() {
    std::cout << "\n";
    print_logo();
    std::cout << "\n";

    std::vector<std::string> ids = provider_ids();
    std::vector<std::string> labels;
    for (size_t i = 0; i < ids.size(); i++) {
        std::string label = provider_label(ids[i]);
        if (i == 0) {
            label += "  Recommended";
        }
        labels.push_back(label);
    }

    int provider_choice = select_option(labels, "Choose a provider:");
    if (provider_choice < 0) {
        print_dim("cancelled");
        return 1;
    }
    std::string provider_id = ids[provider_choice];

    std::cout << "Paste your " << provider_label(provider_id) << " API key: " << std::flush;
    std::string api_key = trim(read_password());
    if (api_key.empty()) {
        print_error("no API key provided");
        return 1;
    }

    std::unique_ptr<Provider> provider = make_provider(provider_id);
    std::vector<ModelSpec> models;
    try {
        models = with_retry("fetching models", [&]() {
            return provider->list_models(api_key);
        });
    } catch (const std::exception& e) {
        std::string msg = e.what();
        if (msg.find("network") != std::string::npos ||
            msg.find("connection") != std::string::npos ||
            msg.find("timeout") != std::string::npos ||
            msg.find("resolve") != std::string::npos) {
            print_error("Can't reach AI models");
        } else {
            print_error(msg);
        }
        return 1;
    }

    // Test free models to verify they work (for Vercel)
    if (provider_id == "vercel") {
        print_dim("verifying free models...");
        for (ModelSpec& model : models) {
            if (model.free && !model.verified_free) {
                std::cout << "  testing " << model.id << "... " << std::flush;
                bool works = provider->test_model(api_key, model);
                model.verified_free = works;
                std::cout << (works ? "ok" : "failed") << "\n";
            }
        }
        rank_models(models);
    }

    std::vector<std::string> labels_models;
    labels_models.reserve(models.size());
    for (const ModelSpec& model : models) {
        labels_models.push_back(format_model(model));
    }

    int model_choice = select_option(labels_models, "Choose a model (type to search):");
    if (model_choice < 0) {
        print_dim("cancelled");
        return 1;
    }

    Config config;
    config.provider = provider_id;
    config.api_key = api_key;
    config.model_id = models[model_choice].id;
    save_config(config);

    print_success("saved " + provider_label(provider_id) + " with " + config.model_id);
    return 0;
}

int run_reset() {
    reset_config();
    print_success("config deleted");
    return 0;
}

}

int run_cli(int argc, wchar_t** argv) {
    cleanup_old_binary();

    std::vector<std::string> args = to_args(argc, argv);
    if (args.empty()) {
        return run_commit("");
    }

    std::vector<std::string> hint_words;
    for (const std::string& arg : args) {
        if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        }
        if (arg == "--version" || arg == "-v") {
            std::cout << "xommit " << XOMMIT_VERSION << "\n";
            return 0;
        }
        if (arg == "--connect") {
            return run_connect();
        }
        if (arg == "--reset") {
            return run_reset();
        }
        if (arg == "--update") {
            update_binary();
            return 0;
        }
        hint_words.push_back(arg);
    }

    return run_commit(join(hint_words, " "));
}

}