// terminal ui. ansi colors, vt sequences, posix console
// print_error, print_success, print_dim, logo
// enable_vt for ansi support (no-op on linux, ansi is native)
// stdin/stdout_is_tty: check if interactive
// terminal_width: for picker viewport
// hide/show_cursor, clear_lines: for spinner/picker animation
// print_logo: ascii art
// read_password: masked input (no echo)
// read_line: normal input
// color_*: ansi escape codes

#include "xommit/ui.h"

#include <clocale>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace xommit {

namespace {

constexpr int LOGO_LINES = 6;
const char* LOGO_COLOR = "\033[38;2;241;78;50m";

struct TermState {
    struct termios old_termios;
    bool valid = false;
};

TermState g_term_state;

} // namespace

void set_utf8() {
    // Initialize UTF-8 locale for proper multibyte/wide conversions
    setlocale(LC_ALL, "C.UTF-8");
}

void enable_vt() {
    // On Linux, ANSI/VT sequences work natively
    // No action needed
}

bool stdin_is_tty() {
    return isatty(STDIN_FILENO) != 0;
}

bool stdout_is_tty() {
    return isatty(STDOUT_FILENO) != 0;
}

int terminal_width() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        return static_cast<int>(ws.ws_col);
    }
    return 80;
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

void print_help_hint() {
    std::cout << color_dim() << "run " << color_green() << "xommit --help" << color_dim() << " for guide" << color_reset() << "\n";
}

std::string read_password() {
    if (!stdin_is_tty()) {
        std::string value = read_line();
        std::cout << "\n" << std::flush;
        return value;
    }

    struct termios old_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    struct termios new_termios = old_termios;
    new_termios.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    std::string value;
    try {
        std::getline(std::cin, value);
        std::cout << "\n" << std::flush;
    } catch (...) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
        throw;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    return value;
}

std::string read_line() {
    std::string value;
    std::getline(std::cin, value);
    if (value.size() >= 3 && value[0] == '\xEF' && value[1] == '\xBB' && value[2] == '\xBF') {
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

} // namespace xommit