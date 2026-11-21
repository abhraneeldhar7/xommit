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

void print_help() {
    std::cout
        << "xommit " << XOMMIT_VERSION << "\n"
        << "\n"
        << "usage:\n"
        << "  xommit [hint]      generate commit messages from staged changes\n"
        << "  xommit --connect   save a provider, API key, and model\n"
        << "  xommit --reset     delete the saved config\n"
        << "  xommit --update    download the latest release\n"
        << "  xommit --version   print the version\n"
        << "  xommit --help      print this help\n";
}

std::vector<std::string> to_args(int argc, wchar_t** argv) {
    std::vector<std::string> result;
    for (int i = 1; i < argc; i++) {
        result.push_back(wide_to_utf8(argv[i]));
    }
    return result;
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

int run_commit(const std::string& hint) {
    print_logo();

    std::string root = git_repo_root();
    std::string raw = git_diff_cached();
    if (trim(raw).empty()) {
        return git_commit_no_args();
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

    Config config = load_config();
    int budget = context_budget_chars(config.model, system_prompt(), hint);
    bool truncated = false;
    std::string context = build_diff_context(sendable, budget, truncated);
    std::string user = build_user_prompt(hint, context);

    std::unique_ptr<Provider> provider = make_provider(config.provider);
    std::string response = with_retry("asking " + config.model.id, [&]() {
        return provider->complete(config.api_key, config.model, system_prompt(), user);
    });

    std::vector<std::string> options = parse_commit_options(response);
    if (options.empty()) {
        throw std::runtime_error("could not read commit messages from the model response");
    }

    int choice = select_option(options, "Choose a commit message:");
    if (choice < 0) {
        print_dim("cancelled");
        return 1;
    }
    return git_commit(options[choice]);
}

int run_connect() {
    print_logo();

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
    std::vector<ModelSpec> models = with_retry("fetching models", [&]() {
        return provider->list_models(api_key);
    });

    std::vector<std::string> labels_models;
    labels_models.reserve(models.size());
    for (const ModelSpec& model : models) {
        labels_models.push_back(format_model(model));
    }

    int model_choice = select_option(labels_models, "Choose a model:");
    if (model_choice < 0) {
        print_dim("cancelled");
        return 1;
    }

    Config config;
    config.provider = provider_id;
    config.api_key = api_key;
    config.model = models[model_choice];
    save_config(config);

    print_success("saved " + provider_label(provider_id) + " with " + config.model.id);
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
