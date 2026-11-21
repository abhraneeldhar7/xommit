#include "xommit/ignore.h"

#include <algorithm>

#include "xommit/util.h"

namespace xommit {

namespace {

const char* BUILTIN_PATTERNS[] = {
    "package-lock.json",
    "pnpm-lock.yaml",
    "yarn.lock",
    "Cargo.lock",
    "go.sum",
    "poetry.lock",
    "composer.lock",
    "Gemfile.lock",
    "*.min.js",
    "*.min.css",
    "*.map",
    "dist/",
    "build/",
    "out/",
    "node_modules/",
    "vendor/",
    "*.png",
    "*.jpg",
    "*.jpeg",
    "*.gif",
    "*.ico",
    "*.webp",
    "*.pdf",
    "*.zip",
    "*.gz",
    "*.7z",
    "*.exe",
    "*.dll",
    "*.so",
    "*.dylib",
    "*.woff",
    "*.woff2",
    "*.ttf",
};

std::string normalize(const std::string& path) {
    std::string result = path;
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

std::string basename(const std::string& path) {
    size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

bool segment_matches(const std::string& pattern, const std::string& path) {
    if (path == pattern) {
        return true;
    }
    size_t start = 0;
    while (start <= path.size()) {
        size_t end = path.find('/', start);
        std::string segment = path.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (segment == pattern) {
            return true;
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return false;
}

}

IgnoreRules::IgnoreRules(const std::string& repo_root) {
    for (const char* pattern : BUILTIN_PATTERNS) {
        patterns_.push_back(pattern);
    }

    std::string path = path_join(repo_root, ".xommitignore");
    if (!file_exists(path)) {
        return;
    }

    std::string text = read_file(path);
    if (starts_with(text, "\xEF\xBB\xBF")) {
        text = text.substr(3);
    }

    for (const std::string& raw_line : split(text, '\n')) {
        std::string line = trim(raw_line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        patterns_.push_back(line);
    }
}

bool IgnoreRules::is_ignored(const std::string& path) const {
    std::string normalized = normalize(path);
    bool ignored = false;
    for (const std::string& raw : patterns_) {
        bool negated = !raw.empty() && raw[0] == '!';
        std::string pattern = negated ? raw.substr(1) : raw;
        if (pattern.empty()) {
            continue;
        }
        if (matches(pattern, normalized)) {
            ignored = !negated;
        }
    }
    return ignored;
}

bool IgnoreRules::matches(const std::string& pattern, const std::string& path) {
    if (ends_with(pattern, "/")) {
        std::string directory = pattern.substr(0, pattern.size() - 1);
        if (path == directory || starts_with(path, directory + "/")) {
            return true;
        }
        return path.find("/" + directory + "/") != std::string::npos;
    }

    if (contains(pattern, "/")) {
        return glob(pattern, path);
    }

    if (glob(pattern, basename(path))) {
        return true;
    }
    return segment_matches(pattern, path);
}

bool IgnoreRules::glob(const std::string& pattern, const std::string& value) {
    size_t pattern_index = 0;
    size_t value_index = 0;
    size_t star_index = std::string::npos;
    size_t match_index = 0;

    while (value_index < value.size()) {
        if (pattern_index < pattern.size() &&
            (pattern[pattern_index] == '?' || pattern[pattern_index] == value[value_index])) {
            pattern_index++;
            value_index++;
        } else if (pattern_index < pattern.size() && pattern[pattern_index] == '*') {
            star_index = pattern_index++;
            match_index = value_index;
        } else if (star_index != std::string::npos) {
            pattern_index = star_index + 1;
            value_index = ++match_index;
        } else {
            return false;
        }
    }

    while (pattern_index < pattern.size() && pattern[pattern_index] == '*') {
        pattern_index++;
    }
    return pattern_index == pattern.size();
}

}
