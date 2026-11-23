// terminal ui. ansi colors, vt sequences, win32 console mode
// print_error, print_success, print_dim, logo
// enable_vt for ansi support on windows

#include "xommit/ui.h"

#include <conio.h>
#include <io.h>
#include <windows.h>

#include <cstdlib>
#include <iostream>

#include "xommit/util.h"

namespace xommit {

namespace {

constexpr int LOGO_LINES = 6;
const char* LOGO_COLOR = "\033[38;2;241;78;50m";

}

void set_utf8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

void enable_vt() {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }
    DWORD mode = 0;
    if (!GetConsoleMode(handle, &mode)) {
        return;
    }
    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

bool stdin_is_tty() {
    return _isatty(_fileno(stdin)) != 0;
}

bool stdout_is_tty() {
    return _isatty(_fileno(stdout)) != 0;
}

int terminal_width() {
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return 80;
    }
    int width = info.srWindow.Right - info.srWindow.Left + 1;
    return width > 0 ? width : 80;
}

void hide_cursor() {
    if (stdout_is_tty()) {
        std::cout << "\033[?25l" << std::flush;
    }
}

void show_cursor() {
    if (stdout_is_tty()) {
        std::cout << "\033[?25h" << std::flush;
    }
}

void clear_lines(int count) {
    if (!stdout_is_tty() || count <= 0) {
        return;
    }
    for (int i = 0; i < count; i++) {
        std::cout << "\033[A\033[2K";
    }
    std::cout << "\r" << std::flush;
}

int print_logo() {
    int width = terminal_width();
    if (stdout_is_tty() && width >= 80) {
        std::cout
            << LOGO_COLOR << "██╗  ██╗" << color_reset() << "  ██████╗  ███╗   ███╗ ███╗   ███╗ ██╗ ████████╗\n"
            << LOGO_COLOR << "╚██╗██╔╝" << color_reset() << " ██╔═══██╗ ████╗ ████║ ████╗ ████║ ██║ ╚══██╔══╝\n"
            << LOGO_COLOR << " ╚███╔╝ " << color_reset() << " ██║   ██║ ██╔████╔██║ ██╔████╔██║ ██║    ██║   \n"
            << LOGO_COLOR << " ██╔██╗ " << color_reset() << " ██║   ██║ ██║╚██╔╝██║ ██║╚██╔╝██║ ██║    ██║   \n"
            << LOGO_COLOR << "██╔╝ ██╗" << color_reset() << " ╚██████╔╝ ██║ ╚═╝ ██║ ██║ ╚═╝ ██║ ██║    ██║   \n"
            << LOGO_COLOR << "╚═╝  ╚═╝" << color_reset() << "  ╚═════╝  ╚═╝     ╚═╝ ╚═╝     ╚═╝ ╚═╝    ╚═╝   \n"
            << color_reset();
        return LOGO_LINES;
    }
    
    return 1;
}

void print_error(const std::string& message) {
    std::cerr << color_red() << "error: " << color_reset() << message << "\n";
}

void print_dim(const std::string& message) {
    std::cout << color_dim() << message << color_reset() << "\n";
}

void print_success(const std::string& message) {
    std::cout << color_green() << message << color_reset() << "\n";
}

std::string read_password() {
    if (!stdin_is_tty()) {
        std::string value = read_line();
        std::cout << "\n" << std::flush;
        return value;
    }

    std::string value;
    while (true) {
        int key = _getch();
        if (key == '\r' || key == '\n') {
            std::cout << "\n" << std::flush;
            break;
        }
        if (key == 3) {
            show_cursor();
            std::cout << "\n" << std::flush;
            std::exit(130);
        }
        if (key == 27) {
            std::cout << "\n" << std::flush;
            return "";
        }
        if (key == 8 || key == 127) {
            if (!value.empty()) {
                value.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        if (key == 0 || key == 224) {
            _getch();
            continue;
        }
        if (key >= 32 && key < 127) {
            value += static_cast<char>(key);
            std::cout << "*" << std::flush;
        }
    }
    return value;
}

std::string read_line() {
    std::string value;
    std::getline(std::cin, value);
    if (starts_with(value, "\xEF\xBB\xBF")) {
        value = value.substr(3);
    }
    return value;
}

const char* color_green() {
    return stdout_is_tty() ? "\033[1;32m" : "";
}

const char* color_dim() {
    return stdout_is_tty() ? "\033[90m" : "";
}

const char* color_red() {
    return stdout_is_tty() ? "\033[1;31m" : "";
}

const char* color_reset() {
    return stdout_is_tty() ? "\033[0m" : "";
}

}
