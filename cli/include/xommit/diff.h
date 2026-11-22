// diff parsing + context budgeting
// DiffFile: path + body (or binary flag)
// parse_diff: git diff --cached -> vector<DiffFile>
// build_diff_context: water-fill algorithm for token budget
// context_budget_chars: fits diff into model context window

#pragma once

#include <string>
#include <vector>

#include "xommit/model.h"

namespace xommit {

struct DiffFile {
    std::string path;
    std::string body;
    bool binary = false;
};

std::vector<DiffFile> parse_diff(const std::string& raw);
std::string build_diff_context(const std::vector<DiffFile>& files, int budget_chars, bool& truncated);
int context_budget_chars(const ModelSpec& model, const std::string& system_prompt, const std::string& hint);
int estimate_tokens(const std::string& text);

}
