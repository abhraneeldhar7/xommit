// google ai studio provider. generativelanguage.googleapis.com
// list models (filter generateContent), generate content
// x-goog-api-key auth

#include "xommit/google.h"

#include <algorithm>
#include <stdexcept>

#include "xommit/http.h"
#include "xommit/json.h"
#include "xommit/util.h"

namespace xommit {

namespace {

std::string response_error(const Json& root) {
    const Json* error = root.find("error");
    if (error == nullptr) {
        return "";
    }
    if (error->is_string()) {
        return error->string;
    }
    return error->get_string("message", error->dump());
}

std::string response_text(const Json& root) {
    const Json* candidates = root.find("candidates");
    if (candidates == nullptr || !candidates->is_array() || candidates->array.empty()) {
        throw std::runtime_error("provider returned no candidates");
    }
    const Json& candidate = candidates->array[0];
    const Json* content = candidate.find("content");
    if (content == nullptr) {
        throw std::runtime_error("provider returned no content");
    }
    const Json* parts = content->find("parts");
    if (parts == nullptr || !parts->is_array()) {
        throw std::runtime_error("provider returned no text");
    }
    std::string text;
    for (const Json& part : parts->array) {
        if (part.is_object()) {
            text += part.get_string("text");
        }
    }
    if (text.empty()) {
        throw std::runtime_error("provider returned empty text");
    }
    return text;
}

}

std::vector<ModelSpec> GoogleProvider::list_models(const std::string& api_key) {
    HttpResponse response = http_request(
        "GET",
        "https://generativelanguage.googleapis.com/v1beta/models?pageSize=1000",
        {"x-goog-api-key: " + api_key, "Accept: application/json"},
        "");
    if (response.status != 200) {
        throw ProviderError(http_error_message(response.status, response.body), response.status);
    }

    Json root = Json::parse(response.body);
    const Json* models = root.find("models");
    if (models == nullptr || !models->is_array()) {
        throw std::runtime_error("provider returned no model list");
    }

    std::vector<ModelSpec> result;
    for (const Json& item : models->array) {
        if (!item.is_object()) {
            continue;
        }

        std::string id = item.get_string("name");
        if (starts_with(id, "models/")) {
            id = id.substr(7);
        }
        if (id.empty()) {
            continue;
        }

        bool supports_generate = true;
        const Json* methods = item.find("supportedGenerationMethods");
        if (methods != nullptr && methods->is_array()) {
            supports_generate = false;
            for (const Json& method : methods->array) {
                if (method.is_string() && method.string == "generateContent") {
                    supports_generate = true;
                    break;
                }
            }
        }
        if (!supports_generate) {
            continue;
        }

        std::string lower = to_lower(id);
        if (contains(lower, "embedding") || contains(lower, "aqa") ||
            contains(lower, "tts") || contains(lower, "audio") || contains(lower, "image") ||
            contains(lower, "vision") || contains(lower, "robotics") || contains(lower, "computer-use") ||
            contains(lower, "transcribe") || contains(lower, "tts")) {
            continue;
        }

        ModelSpec model;
        model.provider = "google";
        model.id = id;
        model.name = item.get_string("displayName", id);
        model.context_window = static_cast<int>(item.get_number("inputTokenLimit"));
        model.max_output_tokens = static_cast<int>(item.get_number("outputTokenLimit"));
        result.push_back(model);
    }

    if (result.empty()) {
        throw std::runtime_error("provider returned no usable text models");
    }
    rank_models(result);
    return result;
}

std::string GoogleProvider::complete(const std::string& api_key,
                                     const ModelSpec& model,
                                     const std::string& system,
                                     const std::string& user) {
    int max_output = model.max_output_tokens > 0 ? std::min(512, model.max_output_tokens) : 512;
    Json body = json_object({
        {"systemInstruction", json_object({
            {"parts", json_array({json_object({{"text", Json(system)}})})},
        })},
        {"contents", json_array({
            json_object({
                {"role", Json("user")},
                {"parts", json_array({json_object({{"text", Json(user)}})})},
            }),
        })},
        {"generationConfig", json_object({
            {"temperature", Json(0.2)},
            {"maxOutputTokens", Json(max_output)},
        })},
    });

    HttpResponse response = http_request(
        "POST",
        "https://generativelanguage.googleapis.com/v1beta/models/" + model.id + ":generateContent",
        {"x-goog-api-key: " + api_key, "Content-Type: application/json", "Accept: application/json"},
        body.dump());
    if (response.status != 200) {
        throw ProviderError(http_error_message(response.status, response.body), response.status);
    }

    Json root = Json::parse(response.body);
    std::string error = response_error(root);
    if (!error.empty()) {
        throw std::runtime_error(error);
    }
    return response_text(root);
}

}
