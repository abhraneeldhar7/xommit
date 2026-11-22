#include "xommit/spinner.h"

#include <chrono>
#include <iostream>

#include "xommit/ui.h"

namespace xommit {

Spinner::Spinner(std::string text) : text_(std::move(text)) {
    if (!stdout_is_tty()) {
        std::cout << text_ << "\n" << std::flush;
        return;
    }
    running_ = true;
    active_ = true;
    thread_ = std::thread(&Spinner::loop, this);
}

Spinner::~Spinner() {
    stop();
}

void Spinner::stop() {
    if (!active_) {
        return;
    }
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
    std::cout << "\r\033[2K" << std::flush;
    active_ = false;
}

void Spinner::loop() {
    const char frames[] = {'o', '0', 'O', '0'};
    int index = 0;
    while (running_) {
        std::cout << "\r" << frames[index] << " " << text_ << std::flush;
        index = (index + 1) % 4;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }
}

}
