#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>

class Logger {
public:
    explicit Logger(size_t maxMessages = 1000)
        : maxMessages_(maxMessages) {}

    void log(const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto t_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_r(&t_c, &tm);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S") << " | " << msg;

        std::lock_guard<std::mutex> lock(mutex_);
        if (messages_.size() >= maxMessages_)
            messages_.erase(messages_.begin());
        messages_.push_back(oss.str());
    }

    std::vector<std::string> get_messages() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return messages_;
    }

    void print() const {
        for (auto& m : get_messages())
            std::cout << m << "\n";
    }

private:
    mutable std::mutex mutex_;
    std::vector<std::string> messages_;
    size_t maxMessages_;
};
