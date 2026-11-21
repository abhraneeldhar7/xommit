#include "xommit/diff.h"

#include <algorithm>
#include <stdexcept>

#include "xommit/util.h"

namespace xommit {

namespace {

bool is_header_line(const std::string& line) {
    return starts_with(line, "index ") ||
           starts_with(line, "--- ") ||
           starts_with(line, "+++ ") ||
           starts_with(line, "new file mode") ||
           starts_with(line, "deleted file mode") ||
           starts_with(line, "old mode") ||
           starts_with(line, "new mode") ||
           starts_with(line, "similarity index") ||
           starts_with(line, "dissimilarity index") ||
           starts_with(line, "rename from") ||
           starts_with(line, "rename to") ||
           starts_with(line, "copy from") ||
           starts_with(line, "copy to");
}

std::string header_path(const std::string& line, bool new_side) {
    std::string rest = line.substr(11);
    size_t split = rest.rfind(" b/");
    if (split == std::string::npos) {
        return "";
    }
    if (new_side) {
        return rest.substr(split + 3);
    }
    std::string path = rest.substr(0, split);
    if (starts_with(path, "a/")) {
        path = path.substr(2);
    }
    return path;
}

std::string truncate_body(const std::string& body, int limit) {
    if (static_cast<int>(body.size()) <= limit) {
        return body;
    }
    size_t cut = body.rfind('\n', static_cast<size_t>(limit));
    if (cut == std::string::npos || cut < 20) {
        cut = static_cast<size_t>(limit);
    }
    std::string result = body.substr(0, cut);
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    result += "\n... truncated";
    return result;
}

}

std::vector<DiffFile> parse_diff(const std::string& raw) {
    std::vector<DiffFile> files;
    std::vector<std::string> lines = split(raw, '\n');
    size_t index = 0;

    while (index < lines.size()) {
        if (!starts_with(lines[index], "diff --git ")) {
            index++;
            continue;
        }

        size_t start = index;
        index++;
        while (index < lines.size() && !starts_with(lines[index], "diff --git ")) {
            index++;
        }

        bool binary = false;
        for (size_t i = start; i < index; i++) {
            if (starts_with(lines[i], "Binary files ") || starts_with(lines[i], "GIT binary patch")) {
                binary = true;
            }
        }

        std::string plus_path = header_path(lines[start], true);
        std::string minus_path = header_path(lines[start], false);
        size_t body_start = start + 1;
        for (size_t i = start + 1; i < index; i++) {
            const std::string& line = lines[i];
            if (starts_with(line, "@@")) {
                body_start = i;
                break;
            }
            if (starts_with(line, "+++ b/")) {
                plus_path = line.substr(6);
            } else if (starts_with(line, "--- a/")) {
                minus_path = line.substr(6);
            } else if (starts_with(line, "rename to ")) {
                plus_path = line.substr(10);
            } else if (starts_with(line, "rename from ")) {
                minus_path = line.substr(12);
            }
        }

        DiffFile file;
        if (!plus_path.empty() && plus_path != "/dev/null") {
            file.path = plus_path;
        } else if (!minus_path.empty() && minus_path != "/dev/null") {
            file.path = minus_path;
        }
        if (file.path.empty()) {
            continue;
        }

        file.binary = binary;
        if (binary) {
            file.body = "(binary file)\n";
        } else {
            for (size_t i = body_start; i < index; i++) {
                if (lines[i].empty()) {
                    continue;
                }
                if (body_start == start + 1 && is_header_line(lines[i])) {
                    continue;
                }
                file.body += lines[i];
                file.body += "\n";
            }
            if (file.body.empty()) {
                file.body = "(no content changes)\n";
            }
        }
        files.push_back(file);
    }

    return files;
}

std::string build_diff_context(const std::vector<DiffFile>& files, int budget_chars, bool& truncated) {
    truncated = false;
    if (files.empty()) {
        return "";
    }
    if (budget_chars <= 0) {
        throw std::runtime_error("model context window is too small");
    }

    std::vector<bool> include(files.size(), false);
    std::vector<int> limit(files.size(), -1);
    std::vector<size_t> pending;
    for (size_t i = 0; i < files.size(); i++) {
        pending.push_back(i);
    }

    size_t remaining = static_cast<size_t>(budget_chars);
    while (!pending.empty()) {
        size_t share = remaining / pending.size();
        if (share < 100) {
            break;
        }

        std::vector<size_t> small;
        std::vector<size_t> large;
        for (size_t i : pending) {
            size_t cost = files[i].path.size() + files[i].body.size() + 20;
            if (cost <= share) {
                small.push_back(i);
            } else {
                large.push_back(i);
            }
        }

        if (small.empty()) {
            for (size_t i : pending) {
                include[i] = true;
                limit[i] = static_cast<int>(share);
            }
            break;
        }

        for (size_t i : small) {
            include[i] = true;
            size_t cost = files[i].path.size() + files[i].body.size() + 20;
            remaining -= std::min(cost, remaining);
        }
        pending = large;
    }

    std::string output;
    bool any_truncated = false;
    for (size_t i = 0; i < files.size(); i++) {
        if (!include[i]) {
            any_truncated = true;
            continue;
        }
        output += "<" + files[i].path + ">\n";
        if (files[i].binary) {
            output += files[i].body;
        } else if (limit[i] >= 0 && static_cast<int>(files[i].body.size()) > limit[i]) {
            output += truncate_body(files[i].body, limit[i]);
            output += "\n";
            any_truncated = true;
        } else {
            output += files[i].body;
        }
        output += "</" + files[i].path + ">\n\n";
    }

    if (output.empty()) {
        throw std::runtime_error("no diff content fits in the model context window");
    }

    truncated = any_truncated;
    if (any_truncated) {
        output = "Note: some diffs were truncated to fit the context window.\n\n" + output;
    }
    return output;
}

int estimate_tokens(const std::string& text) {
    return static_cast<int>((text.size() + 3) / 4);
}

int context_budget_chars(const ModelSpec& model, const std::string& system_prompt, const std::string& hint) {
    int context = model.context_window > 0 ? model.context_window : 32000;
    int output = model.max_output_tokens > 0 ? std::min(512, model.max_output_tokens) : 512;
    int prompt = estimate_tokens(system_prompt) + estimate_tokens(hint) + 100;
    int input = context - output - prompt;
    input = input * 9 / 10;
    if (input < 1000) {
        throw std::runtime_error("model context window is too small");
    }
    return input * 4;
}

}
