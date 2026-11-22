// git wrapper functions
// git_diff_cached: runs git diff --cached, returns raw output
// git_commit: runs git commit -m "message" with proper quoting
// git_commit_no_args: runs git commit (no args) for empty staged
// git_repo_root: git rev-parse --show-toplevel

#pragma once

#include <string>

namespace xommit {

std::string git_diff_cached();
int git_commit(const std::string& message);
int git_commit_no_args();
std::string git_repo_root();

}
