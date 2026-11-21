#include "xommit/provider.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "xommit/google.h"
#include "xommit/json.h"
#include "xommit/util.h"
#include "xommit/vercel.h"

namespace xommit {

namespace {

std::string collapse(const std::string& value) {
    std::string result;
    bool last_space = false;
    for (char c : value) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!last_space && !result.empty()) {
                result += ' ';
            }
            last_space = true;
        } else {
            result += c;
            last_space = false;
        }
    }
    return trim(result);
}

std::string error_detail(const std::string& body) {
    std::string detail;
    try {
        Json root = Json::parse(body);
        const Json* error = root.find("error");
        if (error != nullptr) {
            detail = error->is_string() ? error->string : error->get_string("message");
        }
        if (detail.empty()) {
            detail = root.get_string("message");
        }
    } catch (const std::exception&) {
    }
    if (detail.empty()) {
        detail = body;
    }
    detail = collapse(detail);
    if (detail.size() > 500) {
        detail = detail.substr(0, 500) + "...";
    }
    return detail;
}

std::string format_count(int value) {
    std::ostringstream out;
    if (value >= 1000000) {
        out << std::fixed << std::setprecision(1) << (value / 1000000.0) << "M";
        return out.str();
    }
    if (value >= 1000) {
        out << std::fixed << std::setprecision(1) << (value / 1000.0) << "K";
        return out.str();
    }
    return std::to_string(value);
}

int google_score(const std::string& id) {
    std::string lower = to_lower(id);
    if (contains(lower, "preview") || contains(lower, "exp") || contains(lower, "tts") ||
        contains(lower, "audio") || contains(lower, "image") || contains(lower, "robotics") ||
        contains(lower, "computer-use")) {
        return 100;
    }
    if (contains(lower, "flash-lite")) {
        return 1;
    }
    if (contains(lower, "flash")) {
        return 2;
    }
    if (contains(lower, "pro")) {
        return 3;
    }
    return 50;
}

}

std::unique_ptr<Provider> make_provider(const std::string& id) {
    if (id == "google") {
        return std::make_unique<GoogleProvider>();
    }
    if (id == "vercel") {
        return std::make_unique<VercelProvider>();
    }
    throw std::runtime_error("unknown provider: " + id);
}

std::vector<std::string> provider_ids() {
    return {"google", "vercel"};
}

std::string provider_label(const std::string& id) {
    if (id == "google") {
        return "Google AI Studio";
    }
    if (id == "vercel") {
        return "Vercel AI Gateway";
    }
    return id;
}

void rank_models(std::vector<ModelSpec>& models) {
    if (models.empty()) {
        return;
    }

    if (models.front().provider == "google") {
        std::sort(models.begin(), models.end(), [](const ModelSpec& a, const ModelSpec& b) {
            int score_a = google_score(a.id);
            int score_b = google_score(b.id);
            if (score_a != score_b) {
                return score_a < score_b;
            }
            if (a.context_window != b.context_window) {
                return a.context_window > b.context_window;
            }
            return a.id > b.id;
        });
        for (ModelSpec& model : models) {
            model.free = google_score(model.id) <= 2;
        }
    } else {
        std::sort(models.begin(), models.end(), [](const ModelSpec& a, const ModelSpec& b) {
            if (a.free != b.free) {
                return a.free;
            }
            double cost_a = a.input_price + a.output_price;
            double cost_b = b.input_price + b.output_price;
            if (cost_a != cost_b) {
                return cost_a < cost_b;
            }
            if (a.context_window != b.context_window) {
                return a.context_window > b.context_window;
            }
            return a.id < b.id;
        });
    }

    for (size_t i = 0; i < models.size(); i++) {
        models[i].recommended = i == 0;
    }
}

std::string format_model(const ModelSpec& model) {
    std::string label = model.id;
    label += "  " + format_count(model.context_window) + " ctx";
    label += "  " + format_count(model.max_output_tokens) + " out";
    if (model.free) {
        label += "  Free";
    }
    if (model.recommended) {
        label += "  Recommended";
    }
    return label;
}

// 401, 403, and 429 are final. 5xx and network failures are retried once by the
// caller. Other 4xx responses are reported as they are.
std::string http_error_message(int status, const std::string& body) {
    std::string detail = error_detail(body);
    if (status == 401) {
        return "API key rejected (401). Run xommit --connect.";
    }
    if (status == 403) {
        return "access denied (403). " + detail;
    }
    if (status == 429) {
        return "rate limit or quota exceeded (429). " + detail;
    }
    if (status >= 500) {
        return "provider server error (" + std::to_string(status) + "). " + detail;
    }
    return "provider error (" + std::to_string(status) + "). " + detail;
}

}
