// vercel ai gateway provider. ai-gateway.vercel.sh
// bearer token auth, lists models via /v1/models
// chat/completions endpoint, openai-compatible

#pragma once

#include "xommit/provider.h"

namespace xommit {

class VercelProvider : public Provider {
public:
    std::vector<ModelSpec> list_models(const std::string& api_key) override;
    std::string complete(const std::string& api_key,
                         const ModelSpec& model,
                         const std::string& system,
                         const std::string& user) override;
};

}
