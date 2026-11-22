// vercel ai gateway provider. ai-gateway.vercel.sh
// list models (chat/completions), chat completions
// bearer token auth

#include "xommit/vercel.h"

#include <algorithm>
#include <cstdlib>
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
    const Json* choices = root.find("choices");
    if (choices == nullptr || !choices->is_array() || choices->array.empty()) {
        throw std::runtime_error("provider returned no choices");
    }
    const Json* message = choices->array[0].find("message");
    if (message == nullptr) {
        throw std::runtime_error("provider returned no message");
    }
    const Json* content = message->find("content");
    if (content == nullptr) {
        throw std::runtime_error("provider returned no content");
    }
    if (content->is_string()) {
        if (content->string.empty()) {
            throw std::runtime_error("provider returned empty text");
        }
        return content->string;
    }
    if (content->is_array()) {
        std::string text;
        for (const Json& part : content->array) {
            if (part.is_object()) {
                text += part.get_string("text");
            }
        }
        if (text.empty()) {
            throw std::runtime_error("provider returned empty text");
        }
        return text;
    }
    throw std::runtime_error("provider returned an unknown content type");
}

double read_price(const Json* pricing, const std::string& key, double fallback) {
    if (pricing == nullptr) {
        return fallback;
    }
    const Json* value = pricing->find(key);
    if (value == nullptr) {
        return fallback;
    }
    if (value->type == Json::Type::Number) {
        return value->number;
    }
    if (value->is_string()) {
        char* end = nullptr;
        double parsed = std::strtod(value->string.c_str(), &end);
        if (end != value->string.c_str()) {
            return parsed;
        }
    }
    return fallback;
}

}

std::vector<ModelSpec> VercelProvider::list_models(const std::string& api_key) {
    HttpResponse response = http_request(
        "GET",
        "https://ai-gateway.vercel.sh/v1/models",
        {"Authorization: Bearer " + api_key, "Accept: application/json"},
        "");
    if (response.status != 200) {
        throw ProviderError(http_error_message(response.status, response.body), response.status);
    }

    Json root = Json::parse(response.body);
    const Json* data = root.find("data");
    if (data == nullptr || !data->is_array()) {
        throw std::runtime_error("provider returned no model list");
    }

    std::vector<ModelSpec> result;
    for (const Json& item : data->array) {
        if (!item.is_object()) {
            continue;
        }
        if (item.get_string("type") != "language") {
            continue;
        }

        bool text_output = true;
        const Json* modalities = item.find("modalities");
        if (modalities != nullptr) {
            const Json* output = modalities->find("output");
            if (output != nullptr && output->is_array()) {
                text_output = false;
                for (const Json& modality : output->array) {
                    if (modality.is_string() && modality.string == "text") {
                        text_output = true;
                        break;
                    }
                }
            }
        }
        if (!text_output) {
            continue;
        }

        std::string id = item.get_string("id");
        std::string lower = to_lower(id);
        if (contains(lower, "tts") || contains(lower, "audio") || contains(lower, "image") ||
            contains(lower, "vision") || contains(lower, "robotics") || contains(lower, "computer-use") ||
            contains(lower, "embedding") || contains(lower, "rerank") || contains(lower, "video") ||
            contains(lower, "transcribe") || contains(lower, "speech")) {
            continue;
        }

        ModelSpec model;
        model.provider = "vercel";
        model.id = id;
        if (model.id.empty()) {
            continue;
        }
        model.name = item.get_string("name", model.id);
        model.context_window = static_cast<int>(item.get_number("context_window"));
        model.max_output_tokens = static_cast<int>(item.get_number("max_tokens"));

        const Json* pricing = item.find("pricing");
        model.input_price = read_price(pricing, "input", 1e9);
        model.output_price = read_price(pricing, "output", 1e9);
        model.free = model.input_price == 0.0 && model.output_price == 0.0;

        result.push_back(model);
    }

    if (result.empty()) {
        throw std::runtime_error("provider returned no usable text models");
    }
    rank_models(result);
    return result;
}

std::string VercelProvider::complete(const std::string& api_key,
                                     const ModelSpec& model,
                                     const std::string& system,
                                     const std::string& user) {
    int max_output = model.max_output_tokens > 0 ? std::min(512, model.max_output_tokens) : 512;
    Json body = json_object({
        {"model", Json(model.id)},
        {"messages", json_array({
            json_object({{"role", Json("system")}, {"content", Json(system)}}),
            json_object({{"role", Json("user")}, {"content", Json(user)}}),
        })},
        {"temperature", Json(0.2)},
        {"max_tokens", Json(max_output)},
    });

    HttpResponse response = http_request(
        "POST",
        "https://ai-gateway.vercel.sh/v1/chat/completions",
        {"Authorization: Bearer " + api_key, "Content-Type: application/json", "Accept: application/json"},
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
