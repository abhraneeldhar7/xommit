#include "xommit/prompt.h"

#include <algorithm>
#include <cctype>

#include "xommit/json.h"
#include "xommit/util.h"

namespace xommit {

namespace {

const char* SYSTEM_PROMPT =
    "You write git commit messages in Conventional Commits format.\n"
    "\n"
    "Return exactly three different options as JSON and nothing else:\n"
    "{\"commits\":[\"<option 1>\",\"<option 2>\",\"<option 3>\"]}\n"
    "\n"
    "Rules:\n"
    "- Each option is a single line: type(scope): subject\n"
    "- type is one of: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert\n"
    "- scope is optional and only when the diff makes it obvious\n"
    "- subject is lowercase, imperative, and has no trailing period\n"
    "- keep each option at or under 72 characters\n"
    "- describe the actual change in the diff, not the user's wording\n"
    "- if the user hint and the diff disagree, trust the diff\n"
    "- no body, no quotes, no markdown, no explanations\n"
    "- if the diff is truncated, use the visible changes only\n";

std::string collapse_spaces(const std::string& value) {
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

void append_unique(std::vector<std::string>& values, const std::string& value) {
    std::string clean = sanitize_message(value);
    if (clean.empty()) {
        return;
    }
    if (std::find(values.begin(), values.end(), clean) == values.end()) {
        values.push_back(clean);
    }
}

}

const char* system_prompt() {
    return SYSTEM_PROMPT;
}

std::string build_user_prompt(const std::string& hint, const std::string& diff_context) {
    std::string result = "User hint: ";
    result += hint.empty() ? "(none)" : hint;
    result += "\n\n";
    result += diff_context;
    return result;
}

std::string sanitize_message(const std::string& value) {
    std::string result = replace_all(value, "\r", " ");
    result = replace_all(result, "\n", " ");
    result = collapse_spaces(result);
    if (result.size() >= 2 && result.front() == '"' && result.back() == '"') {
        result = result.substr(1, result.size() - 2);
    }
    if (!result.empty() && result.back() == '.') {
        result.pop_back();
    }
    if (result.size() > 72) {
        size_t cut = result.rfind(' ', 72);
        if (cut == std::string::npos || cut < 20) {
            cut = 72;
        }
        while (cut < result.size() &&
               (static_cast<unsigned char>(result[cut]) & 0xC0) == 0x80) {
            cut--;
        }
        result = trim(result.substr(0, cut));
    }
    return result;
}

std::vector<std::string> parse_commit_options(const std::string& response) {
    std::vector<std::string> result;

    size_t start = response.find_first_of("{[");
    size_t end = response.find_last_of("}]");
    if (start != std::string::npos && end != std::string::npos && end > start) {
        try {
            Json root = Json::parse(response.substr(start, end - start + 1));
            if (root.is_object()) {
                const Json* commits = root.find("commits");
                if (commits != nullptr && commits->is_array()) {
                    for (const Json& item : commits->array) {
                        if (item.is_string()) {
                            append_unique(result, item.string);
                        }
                    }
                }
            } else if (root.is_array()) {
                for (const Json& item : root.array) {
                    if (item.is_string()) {
                        append_unique(result, item.string);
                    }
                }
            }
        } catch (const std::exception&) {
        }
    }

    if (result.empty()) {
        for (const std::string& raw_line : split(response, '\n')) {
            std::string line = trim(raw_line);
            if (line.empty()) {
                continue;
            }
            if (starts_with(line, "```") || starts_with(line, "{") || starts_with(line, "[") ||
                starts_with(line, "\"")) {
                continue;
            }
            if (line[0] == '-' || line[0] == '*' || line[0] == '+') {
                line = trim(line.substr(1));
            } else if (std::isdigit(static_cast<unsigned char>(line[0]))) {
                size_t dot = line.find_first_of(".)");
                if (dot != std::string::npos) {
                    line = trim(line.substr(dot + 1));
                }
            }
            append_unique(result, line);
        }
    }

    if (result.size() > 3) {
        result.resize(3);
    }
    return result;
}

}
