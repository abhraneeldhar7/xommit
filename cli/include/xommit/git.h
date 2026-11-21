#pragma once

#include <string>

namespace xommit {

std::string git_diff_cached();
int git_commit(const std::string& message);
int git_commit_no_args();
std::string git_repo_root();

}
