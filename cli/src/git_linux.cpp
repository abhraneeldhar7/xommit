// git wrapper. runs git commands (diff, commit, rev-parse, log)
// uses posix_spawn for proper linux process handling

#include "xommit/git.h"

#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "xommit/util.h"

namespace xommit {

namespace {

struct ProcessResult {
    int exit_code = 0;
    std::string output;
    std::string error;
};

void read_fd(int fd, std::string& target) {
    char buffer[4096];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        target.append(buffer, n);
    }
}

ProcessResult run_capture(const std::vector<std::string>& args) {
    int out_pipe[2];
    int err_pipe[2];
    if (pipe(out_pipe) != 0 || pipe(err_pipe) != 0) {
        throw std::runtime_error("cannot create pipes: " + std::string(std::strerror(errno)));
    }

    std::vector<const char*> argv;
    argv.reserve(args.size() + 1);
    argv.push_back("git");
    for (const std::string& arg : args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, out_pipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, err_pipe[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, out_pipe[0]);
    posix_spawn_file_actions_addclose(&actions, err_pipe[0]);
    posix_spawn_file_actions_addclose(&actions, out_pipe[1]);
    posix_spawn_file_actions_addclose(&actions, err_pipe[1]);

    pid_t pid;
    int spawn_result = posix_spawnp(&pid, "git", &actions, nullptr, const_cast<char* const*>(argv.data()), environ);
    posix_spawn_file_actions_destroy(&actions);

    close(out_pipe[1]);
    close(err_pipe[1]);

    if (spawn_result != 0) {
        close(out_pipe[0]);
        close(err_pipe[0]);
        if (spawn_result == ENOENT) {
            throw std::runtime_error("git is not installed or added to PATH");
        }
        throw std::runtime_error("cannot run git: " + std::string(std::strerror(spawn_result)));
    }

    ProcessResult result;
    std::thread out_thread(read_fd, out_pipe[0], std::ref(result.output));
    std::thread err_thread(read_fd, err_pipe[0], std::ref(result.error));

    int status;
    waitpid(pid, &status, 0);

    out_thread.join();
    err_thread.join();

    close(out_pipe[0]);
    close(err_pipe[0]);

    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    } else {
        result.exit_code = -1;
    }

    return result;
}

int run_git_inherit(const std::vector<std::string>& args) {
    std::vector<const char*> argv;
    argv.reserve(args.size() + 1);
    argv.push_back("git");
    for (const std::string& arg : args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);

    pid_t pid;
    int spawn_result = posix_spawnp(&pid, "git", nullptr, nullptr, const_cast<char* const*>(argv.data()), environ);
    if (spawn_result != 0) {
        if (spawn_result == ENOENT) {
            throw std::runtime_error("git is not installed or added to PATH");
        }
        throw std::runtime_error("cannot run git: " + std::string(std::strerror(spawn_result)));
    }

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

std::string quote_argument(const std::string& value) {
    if (value.empty()) {
        return "''";
    }
    if (value.find_first_of(" \t\n\v\"'`$\\") == std::string::npos) {
        return value;
    }

    std::string result = "'";
    for (char c : value) {
        if (c == '\'') {
            result += "'\\''";
        } else {
            result += c;
        }
    }
    result += "'";
    return result;
}

} // namespace

std::string git_diff_cached() {
    ProcessResult result = run_capture({
        "-c", "core.quotepath=false",
        "diff", "--cached", "--no-color", "--no-ext-diff", "--no-textconv", "-M"
    });
    if (result.exit_code != 0) {
        std::string message = trim(result.error);
        if (message.empty()) {
            message = "git diff failed";
        }
        throw std::runtime_error(message);
    }
    return result.output;
}

int git_commit(const std::string& message) {
    std::string cmd = "git commit -m " + quote_argument(message);
    std::vector<std::string> args = {"commit", "-m", message};
    return run_git_inherit(args);
}

int git_commit_no_args() {
    return run_git_inherit({"commit"});
}

std::string git_repo_root() {
    ProcessResult result = run_capture({"rev-parse", "--show-toplevel"});
    if (result.exit_code != 0) {
        std::string message = trim(result.error);
        if (message.empty()) {
            message = "not a git repository";
        }
        throw std::runtime_error(message);
    }
    return trim(result.output);
}

} // namespace xommit