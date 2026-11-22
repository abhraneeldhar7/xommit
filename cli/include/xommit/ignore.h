// ignore rules. built-in patterns + .xommitignore
// IgnoreRules loads patterns, checks is_ignored(path)
// dir/ matches directory, ! negates, * and ? wildcards

#pragma once

#include <string>
#include <vector>

namespace xommit {

class IgnoreRules {
public:
    explicit IgnoreRules(const std::string& repo_root);
    bool is_ignored(const std::string& path) const;

private:
    std::vector<std::string> patterns_;

    static bool matches(const std::string& pattern, const std::string& path);
    static bool glob(const std::string& pattern, const std::string& value);
};

}
