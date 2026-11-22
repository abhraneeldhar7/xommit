// entry point. sets up utf8 and vt, calls run_cli, catches exceptions

#include <exception>

#include "xommit/cli.h"
#include "xommit/ui.h"

int wmain(int argc, wchar_t** argv) {
    xommit::set_utf8();
    xommit::enable_vt();

    try {
        return xommit::run_cli(argc, argv);
    } catch (const std::exception& error) {
        xommit::print_error(error.what());
        return 1;
    }
}
