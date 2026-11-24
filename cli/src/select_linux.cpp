// interactive picker. shows 3 commit options, arrow keys to navigate
// type-to-filter, pgup/pgdn, home/end, esc=cancel
// falls back to numbered list in non-tty

#include "xommit/select.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "xommit/ui.h"
#include "xommit/util.h"

namespace xommit {

namespace {

constexpr int VISIBLE_ROWS = 10;

std::string truncate_label(const std::string& label, int width) {
    if (width < 20) {
        width = 20;
    }
    if (static_cast<int>(label.size()) <= width) {
        return label;
    }
    if (width <= 3) {
        return label.substr(0, static_cast<size_t>(width));
    }
    return label.substr(0, static_cast<size_t>(width - 3)) + "...";
}

int select_numbered(const std::vector<std::string>& options, const std::string& title) {
    if (!title.empty()) {
        std::cout << title << "\n";
    }
    for (size_t i = 0; i < options.size(); i++) {
        std::cout << "  " << (i + 1) << ". " << options[i] << "\n";
    }
    std::cout << "Select [1-" << options.size() << "]: " << std::flush;
    std::string line = read_line();
    if (!stdin_is_tty()) {
        std::cout << "\n" << std::flush;
    }
    int choice = std::atoi(line.c_str());
    if (choice < 1 || choice > static_cast<int>(options.size())) {
        return -1;
    }
    return choice - 1;
}

int read_key() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) {
        return -1;
    }
    if (c == '\x1b') {
        char seq[2];
        if (read(STDIN_FILENO, seq, 1) != 1) return 27;
        if (seq[0] != '[') return 27;
        if (read(STDIN_FILENO, seq + 1, 1) != 1) return 27;
        switch (seq[1]) {
            case 'A': return 1000 + 72; // up
            case 'B': return 1000 + 80; // down
            case 'C': return 1000 + 77; // right
            case 'D': return 1000 + 75; // left
            case 'H': return 1000 + 71; // home
            case 'F': return 1000 + 79; // end
            case '5': 
                if (read(STDIN_FILENO, seq, 1) == 1 && seq[0] == '~') return 1000 + 73; // pgup
                return 27;
            case '6':
                if (read(STDIN_FILENO, seq, 1) == 1 && seq[0] == '~') return 1000 + 81; // pgdn
                return 27;
            case '3':
                if (read(STDIN_FILENO, seq, 1) == 1 && seq[0] == '~') return 8; // del
                return 27;
            default: return 27;
        }
    }
    return static_cast<unsigned char>(c);
}

int select_interactive(const std::vector<std::string>& options, const std::string& title) {
    int selected = 0;
    int offset = 0;
    int drawn = 0;
    std::string filter;
    std::vector<int> visible;

    auto rebuild = [&]() {
        visible.clear();
        std::string needle = to_lower(filter);
        for (int i = 0; i < static_cast<int>(options.size()); i++) {
            if (needle.empty() || to_lower(options[i]).find(needle) != std::string::npos) {
                visible.push_back(i);
            }
        }
        if (visible.empty()) {
            selected = 0;
        } else if (selected >= static_cast<int>(visible.size())) {
            selected = static_cast<int>(visible.size()) - 1;
        }
        if (selected < 0) {
            selected = 0;
        }
        if (offset > selected) {
            offset = selected;
        }
        if (selected >= offset + VISIBLE_ROWS) {
            offset = selected - VISIBLE_ROWS + 1;
        }
        if (offset > static_cast<int>(visible.size()) - VISIBLE_ROWS) {
            offset = std::max(0, static_cast<int>(visible.size()) - VISIBLE_ROWS);
        }
    };

    auto draw = [&]() {
        if (drawn > 0) {
            clear_lines(drawn);
        }
        drawn = 0;

        int width = terminal_width();
        if (!title.empty()) {
            std::cout << title << "\n";
            drawn++;
        }
        std::cout << color_dim() << "Type to search: " << color_reset() << filter << "\n";
        drawn++;

        if (visible.empty()) {
            std::cout << "  no matches\n";
            drawn++;
        } else {
            int end = std::min(static_cast<int>(visible.size()), offset + VISIBLE_ROWS);
            for (int i = offset; i < end; i++) {
                std::string label = truncate_label(options[visible[i]], width - 4);
                if (i == selected) {
                    std::cout << color_green() << "  > " << label << color_reset() << "\n";
                } else {
                    std::cout << "    " << label << "\n";
                }
                drawn++;
            }
        }

        std::cout << color_dim() << "  up/down navigate, type to filter, enter select, esc cancel"
                  << color_reset() << "\n";
        drawn++;

        std::cout << color_dim() << "  (" << (visible.empty() ? 0 : selected + 1) << "/"
                  << visible.size() << ")" << color_reset() << "\n";
        drawn++;
    };

    hide_cursor();
    rebuild();
    draw();

    while (true) {
        int key = read_key();
        if (key >= 1000) {
            int special = key - 1000;
            if (visible.empty()) {
                draw();
                continue;
            }
            if (special == 72) {
                if (!visible.empty() && selected > 0) {
                    selected--;
                }
            } else if (special == 80) {
                if (!visible.empty() && selected < static_cast<int>(visible.size()) - 1) {
                    selected++;
                }
            } else if (special == 73) {
                selected = std::max(0, selected - VISIBLE_ROWS);
            } else if (special == 81) {
                selected = std::min(static_cast<int>(visible.size()) - 1, selected + VISIBLE_ROWS);
            } else if (special == 71) {
                selected = 0;
            } else if (special == 79) {
                selected = visible.empty() ? 0 : static_cast<int>(visible.size()) - 1;
            }
            if (selected < offset) {
                offset = selected;
            }
            if (selected >= offset + VISIBLE_ROWS) {
                offset = selected - VISIBLE_ROWS + 1;
            }
            draw();
            continue;
        }

        if (key == 3 || key == 27) {
            clear_lines(drawn);
            show_cursor();
            return -1;
        }
        if (key == '\r' || key == '\n') {
            int result = visible.empty() ? -1 : visible[selected];
            clear_lines(drawn);
            show_cursor();
            return result;
        }
        if (key == 8 || key == 127) {
            if (!filter.empty()) {
                filter.pop_back();
                selected = 0;
                offset = 0;
                rebuild();
                draw();
            }
            continue;
        }
        if (key >= 32 && key < 127) {
            filter += static_cast<char>(key);
            selected = 0;
            offset = 0;
            rebuild();
            draw();
        }
    }
}

}

int select_option(const std::vector<std::string>& options, const std::string& title) {
    if (options.empty()) {
        return -1;
    }
    if (!stdin_is_tty() || !stdout_is_tty()) {
        return select_numbered(options, title);
    }
    return select_interactive(options, title);
}

} // namespace xommit