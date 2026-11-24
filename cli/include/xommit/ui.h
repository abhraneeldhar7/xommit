// terminal ui helpers
// set_utf8: set console output to utf8
// enable_vt: enable ansi vt sequences on windows
// stdin/stdout_is_tty: check if interactive
// terminal_width: for picker viewport
// hide/show_cursor, clear_lines: for spinner/picker animation
// print_logo: ascii art
// print_error/dim/success: colored output
// read_password: masked input (no echo)
// read_line: normal input
// color_*: ansi escape codes

#pragma once

#include <string>

namespace xommit {

void set_utf8();
void enable_vt();
bool stdin_is_tty();
bool stdout_is_tty();
int terminal_width();

void hide_cursor();
void show_cursor();
void clear_lines(int count);
int print_logo();

void print_error(const std::string& message);
void print_dim(const std::string& message);
void print_success(const std::string& message);
void print_help_hint();

std::string read_password();
std::string read_line();

const char* color_green();
const char* color_dim();
const char* color_red();
const char* color_reset();

}
