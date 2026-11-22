// main entry point. called from main.cpp
// parses args, dispatches to --connect, --update, --reset, --model, --uninstall, or commit flow

#pragma once

namespace xommit {

int run_cli(int argc, wchar_t** argv);

}
