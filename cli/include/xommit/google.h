// google ai studio provider. generativelanguage.googleapis.com
// x-goog-api-key header, lists models with generateContent
// generateContent endpoint, systemInstruction + contents

#pragma once

#include "xommit/provider.h"

namespace xommit {

class GoogleProvider : public Provider {
public:
    std::vector<ModelSpec> list_models(const std::string& api_key) override;
    std::string complete(const std::string& api_key,
                         const ModelSpec& model,
                         const std::string& system,
                         const std::string& user) override;
};

}
