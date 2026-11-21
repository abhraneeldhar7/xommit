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

std::string read_password();
std::string read_line();

const char* color_green();
const char* color_dim();
const char* color_red();
const char* color_reset();

}
