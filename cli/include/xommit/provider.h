// provider interface + registry

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "xommit/model.h"

namespace xommit {

class ProviderError : public std::runtime_error {
public:
    ProviderError(std::string message, int status)
        : std::runtime_error(std::move(message)), status_(status) {}

    int status() const { return status_; }

    bool retryable() const { return status_ >= 500; }

private:
    int status_;
};

class Provider {
public:
    virtual ~Provider() = default;

    virtual std::vector<ModelSpec> list_models(const std::string& api_key) = 0;
    virtual std::string complete(const std::string& api_key,
                                 const ModelSpec& model,
                                 const std::string& system,
                                 const std::string& user) = 0;

    virtual bool test_model(const std::string& api_key, const ModelSpec& model) {
        try {
            complete(api_key, model, "Test", "Reply with 'ok'");
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
};

std::unique_ptr<Provider> make_provider(const std::string& id);
std::vector<std::string> provider_ids();
std::string provider_label(const std::string& id);
void rank_models(std::vector<ModelSpec>& models);
std::string format_model(const ModelSpec& model);
std::string http_error_message(int status, const std::string& body);

}