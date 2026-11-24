// spinner animation

#pragma once

#include <atomic>
#include <string>
#include <thread>

namespace xommit {

class Spinner {
public:
    explicit Spinner(std::string text);
    ~Spinner();

    void stop();

private:
    std::string text_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    bool active_ = false;

    void loop();
};

}