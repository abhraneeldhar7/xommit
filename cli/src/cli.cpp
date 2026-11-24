// main cli logic. handles all commands (--connect, --update, --reset, etc)
// runs the commit flow: git diff -> filter -> ai -> picker -> git commit

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
int run_connect(const std::string& hint = "", bool show_logo = true);

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

// ANSI hyperlink: \033]8;;URL\033\text\033]8;;
inline std::string style_link(const std::string& text, const std::string& url) {
    return stdout_is_tty() ? ("\033]8;;" + url + "\033\\" + text + "\033]8;;\033\\") : text;
}

// Links
const char* LINK_XOMMIT = XOMMIT_HOMEPAGE;

void print_help() {
    std::cout << "\n";
    print_logo();
    std::cout << "\n";

    std::cout << style_dim("Hello, I made ") << style_link(style_cyan("xommit"), LINK_XOMMIT) << style_dim(" to turn your vague commits into conventional like a pro :D") << "\n";
    std::cout << "\n";

    std::cout << "version " << style_bold(XOMMIT_VERSION) << "\n";
    std::cout << "\n";

    std::cout << style_dim("xommit") << " " << style_green("<your message (optional)>") << "\n";
    std::cout << style_dim("xommit") << " " << style_green("--connect") << "              save a provider, API key, and model\n";
    std::cout << style_dim("xommit") << " " << style_green("--model") << "                change model for current provider\n";
    std::cout << style_dim("xommit") << " " << style_green("--reset") << "                delete the saved config\n";
    std::cout << style_dim("xommit") << " " << style_green("--update") << "               download the latest release\n";
    std::cout << style_dim("xommit") << " " << style_green("--uninstall") << "            remove binary, config, and cache\n";
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
    } catch (const std::exception& error) {
        if (config_exists()) {
            print_error(error.what());
            return 1;
        }
        std::cout << style_dim("Connect your AI provider to begin") << "\n\n";
        return run_connect(hint, false);
    }

    ModelSpec model;
    try {
        model = resolve_model(config.provider, config.api_key, config.model_id);
    } catch (const std::exception& e) {
        print_error("failed to resolve model: " + std::string(e.what()));
        return 1;
    }

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
    // Sanitize the chosen message to avoid git pathspec issues
    std::string msg = options[choice];
    msg = replace_all(msg, "\n", " ");
    msg = replace_all(msg, "\r", " ");
    // collapse multiple spaces
    std::string cleaned;
    bool last_space = false;
    for (char c : msg) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!last_space && !cleaned.empty()) {
                cleaned += ' ';
            }
            last_space = true;
        } else {
            cleaned += c;
            last_space = false;
        }
    }
    msg = trim(cleaned);
    return git_commit(msg);
}

int run_connect(const std::string& hint, bool show_logo) {
    if (show_logo) {
        std::cout << "\n";
        print_logo();
        std::cout << "\n";
    }

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

    std::vector<std::string> labels_models;
    labels_models.reserve(models.size());
    for (const ModelSpec& model : models) {
        labels_models.push_back(format_model(model));
    }

    int model_choice = -1;
    int drawn_after_test = 0;
    ModelSpec selected_model;
    while (true) {
        if (drawn_after_test > 0) {
            clear_lines(drawn_after_test);
            drawn_after_test = 0;
        }
        model_choice = select_option(labels_models, "Choose a model (type to search):");
        if (model_choice < 0) {
            print_dim("cancelled");
            return 1;
        }

        selected_model = models[model_choice];
        std::cout << "  testing " << selected_model.id << "... " << std::flush;
        drawn_after_test = 1;

        try {
            provider->complete(api_key, selected_model, "Test", "hi");
            std::cout << "ok\n";
            drawn_after_test++;
            break;
        } catch (const std::exception& e) {
            std::string msg = e.what();
            std::cout << "failed\n";
            drawn_after_test++;
            if (drawn_after_test > 0) {
                clear_lines(drawn_after_test);
                drawn_after_test = 0;
            }
            std::cout << "\n";
            std::cout << style_green(provider_label(provider_id)) << " says: " << msg << "\n\n";
            // Continue loop to re-show selection
        }
    }

    Config config;
    config.provider = provider_id;
    config.api_key = api_key;
    config.model_id = selected_model.id;
    save_config(config);

    print_success("saved " + provider_label(provider_id) + " with " + config.model_id);

    // If called from commit flow with a hint, continue to commit
    if (!hint.empty()) {
        // Clear the success message and run commit
        clear_lines(2); // success message + blank line
        return run_commit(hint);
    }
    return 0;
}

int run_model_select() {
    std::cout << "\n";
    print_logo();
    std::cout << "\n";

    Config config;
    try {
        config = load_config();
    } catch (const std::exception& error) {
        if (config_exists()) {
            print_error(error.what());
            return 1;
        }
        std::cout << style_dim("Connect your AI provider first") << "\n\n";
        return run_connect("", true);
    }

    std::unique_ptr<Provider> provider = make_provider(config.provider);

    // Fetch models for listing (key already verified in --connect)
    std::vector<ModelSpec> models;
    try {
        models = with_retry("fetching models", [&]() {
            return provider->list_models(config.api_key);
        });
    } catch (const std::exception& e) {
        std::string msg = e.what();
        if (msg.find("network") != std::string::npos ||
            msg.find("connection") != std::string::npos ||
            msg.find("timeout") != std::string::npos ||
            msg.find("resolve") != std::string::npos) {
            print_error("Can't reach AI models");
        } else if (msg.find("401") != std::string::npos || msg.find("403") != std::string::npos) {
            std::cout << style_dim("API key invalid or expired") << "\n\n";
            return run_connect("", true);
        } else {
            print_error(msg);
        }
        return 1;
    }

    std::vector<std::string> labels_models;
    labels_models.reserve(models.size());
    for (const ModelSpec& model : models) {
        labels_models.push_back(format_model(model));
    }

    int model_choice = -1;
    int drawn_after_test = 0;
    ModelSpec selected_model;
    while (true) {
        if (drawn_after_test > 0) {
            clear_lines(drawn_after_test);
            drawn_after_test = 0;
        }
        model_choice = select_option(labels_models, "Choose a model (type to search):");
        if (model_choice < 0) {
            print_dim("cancelled");
            return 1;
        }

        selected_model = models[model_choice];
        std::cout << "  testing " << selected_model.id << "... " << std::flush;
        drawn_after_test = 1;

        try {
            provider->complete(config.api_key, selected_model, "Test", "hi");
            std::cout << "ok\n";
            drawn_after_test++;
            break;
        } catch (const std::exception& e) {
            std::string msg = e.what();
            std::cout << "failed\n";
            drawn_after_test++;
            if (drawn_after_test > 0) {
                clear_lines(drawn_after_test);
                drawn_after_test = 0;
            }
            std::cout << "\n";
            std::cout << style_green(provider_label(config.provider)) << " says: " << msg << "\n\n";
        }
    }

    config.model_id = selected_model.id;
    save_config(config);

    print_success("saved " + provider_label(config.provider) + " with " + config.model_id);
    return 0;
}

int run_reset() {
    reset_config();
    print_success("Your saved data is deleted");
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
        if (arg == "--uninstall") {
            uninstall();
            return 0;
        }
        if (arg == "--model") {
            return run_model_select();
        }
        if (arg.rfind("--", 0) == 0) {
            // Unknown flag starting with --
            std::cout << "\n";
            print_logo();
            std::cout << "\n";
            std::cout << style_dim("Command not recognized, use ") << style_green("--help") << "\n\n";
            return 1;
        }
        hint_words.push_back(arg);
    }

    return run_commit(join(hint_words, " "));
}

}