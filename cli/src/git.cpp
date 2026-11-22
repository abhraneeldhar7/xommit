// git wrapper. runs git commands (diff, commit, rev-parse, log)
// uses CreateProcess for proper windows quoting

#include "xommit/git.h"

#include <windows.h>

#include <process.h>

#include <functional>
#include <stdexcept>
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

void read_pipe(HANDLE pipe, std::string& target) {
    char buffer[4096];
    DWORD read = 0;
    while (ReadFile(pipe, buffer, sizeof(buffer), &read, nullptr) && read > 0) {
        target.append(buffer, read);
    }
}

ProcessResult run_capture(const std::wstring& command) {
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;

    HANDLE output_read = nullptr;
    HANDLE output_write = nullptr;
    HANDLE error_read = nullptr;
    HANDLE error_write = nullptr;

    if (!CreatePipe(&output_read, &output_write, &attributes, 0) ||
        !CreatePipe(&error_read, &error_write, &attributes, 0)) {
        throw std::runtime_error("cannot create process pipes");
    }

    SetHandleInformation(output_read, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(error_read, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = output_write;
    startup.hStdError = error_write;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION process{};
    std::vector<wchar_t> buffer(command.begin(), command.end());
    buffer.push_back(L'\0');

    BOOL started = CreateProcessW(nullptr, buffer.data(), nullptr, nullptr, TRUE,
                                  CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    if (!started) {
        DWORD code = GetLastError();
        CloseHandle(output_read);
        CloseHandle(output_write);
        CloseHandle(error_read);
        CloseHandle(error_write);
        if (code == ERROR_FILE_NOT_FOUND) {
            throw std::runtime_error("git is not installed or not on PATH");
        }
        throw std::runtime_error("cannot run git: " + windows_error(code));
    }

    CloseHandle(output_write);
    CloseHandle(error_write);

    ProcessResult result;
    std::thread output_thread(read_pipe, output_read, std::ref(result.output));
    std::thread error_thread(read_pipe, error_read, std::ref(result.error));

    WaitForSingleObject(process.hProcess, INFINITE);
    output_thread.join();
    error_thread.join();

    DWORD exit_code = 0;
    GetExitCodeProcess(process.hProcess, &exit_code);
    result.exit_code = static_cast<int>(exit_code);

    CloseHandle(process.hProcess);
    CloseHandle(process.hThread);
    CloseHandle(output_read);
    CloseHandle(error_read);

    return result;
}

int run_git(const wchar_t* const* arguments) {
    intptr_t result = _wspawnvp(_P_WAIT, L"git", arguments);
    if (result == -1) {
        throw std::runtime_error("git is not installed or not on PATH");
    }
    return static_cast<int>(result);
}

// Quote one argument for the Windows command line parser.
std::wstring quote_argument(const std::wstring& value) {
    if (value.empty()) {
        return L"\"\"";
    }
    if (value.find_first_of(L" \t\n\v\"") == std::wstring::npos) {
        return value;
    }

    std::wstring result = L"\"";
    size_t backslashes = 0;
    for (wchar_t c : value) {
        if (c == L'\\') {
            backslashes++;
        } else if (c == L'"') {
            result.append(backslashes * 2 + 1, L'\\');
            result += L'"';
            backslashes = 0;
        } else {
            result.append(backslashes, L'\\');
            backslashes = 0;
            result += c;
        }
    }
    result.append(backslashes * 2, L'\\');
    result += L'"';
    return result;
}

int run_git_inherit(const std::wstring& command) {
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);

    PROCESS_INFORMATION process{};
    std::vector<wchar_t> buffer(command.begin(), command.end());
    buffer.push_back(L'\0');

    if (!CreateProcessW(nullptr, buffer.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr,
                        &startup, &process)) {
        DWORD code = GetLastError();
        if (code == ERROR_FILE_NOT_FOUND) {
            throw std::runtime_error("git is not installed or not on PATH");
        }
        throw std::runtime_error("cannot run git: " + windows_error(code));
    }

    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hProcess);
    CloseHandle(process.hThread);
    return static_cast<int>(exit_code);
}

}

std::string git_diff_cached() {
    ProcessResult result = run_capture(
        L"git -c core.quotepath=false diff --cached --no-color --no-ext-diff --no-textconv -M");
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
    std::wstring command = L"git commit -m " + quote_argument(utf8_to_wide(message));
    return run_git_inherit(command);
}

int git_commit_no_args() {
    const wchar_t* arguments[] = {L"git", L"commit", nullptr};
    return run_git(arguments);
}

std::string git_repo_root() {
    ProcessResult result = run_capture(L"git rev-parse --show-toplevel");
    if (result.exit_code != 0) {
        std::string message = trim(result.error);
        if (message.empty()) {
            message = "not a git repository";
        }
        throw std::runtime_error(message);
    }
    return trim(result.output);
}

}
