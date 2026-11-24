// entry point for linux. sets up utf8, calls run_cli, catches exceptions

#include <exception>

#include "xommit/cli.h"
#include "xommit/ui.h"

int main(int argc, char** argv) {
    xommit::set_utf8();
    xommit::enable_vt();

    // Convert char** argv to wchar_t** for run_cli compatibility
    std::vector<std::wstring> wargs;
    wargs.reserve(argc);
    for (int i = 0; i < argc; i++) {
        wargs.push_back(std::wstring(argv[i].begin(), argv[i].end()));
    }
    std::vector<wchar_t*> wargv;
    wargv.reserve(wargs.size());
    for (auto& ws : wargs) {
        wargv.push_back(ws.data());
    }

    try {
        return xommit::run_cli(argc, wargv.data());
    } catch (const std::exception& error) {
        xommit::print_error(error.what());
        return 1;
    }
}