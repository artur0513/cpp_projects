#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>
#include "3dMath/3dMath.h"

namespace Syslog {

    enum : char {
        FATAL = 'f',
        ERROR = 'e',
        WARNING = 'w',
        INFO = 'i',
        SUCSSES = 's',
        DEBUG = 'd'
    };

    class Message {
    private:
        char type;
        m3d::vec4f color;
        std::chrono::time_point<std::chrono::system_clock> time;
        std::string message;
        /* Probably add stacktrace support for fatal, error and debug messages */

        /* Summary is string that contains time, type and message together.
         * summaryColors is needed for terminals that support colors */
        std::string summary, summaryColors;

        void createSummary() noexcept;

    public:
        Message(const std::string& _message, char _type = INFO);

        const std::string& getSummary() const noexcept;

        const std::string& getSummaryColors() const noexcept;

        const char getType() const noexcept;

        m3d::vec4f getColor() const noexcept;
    };

    void log(const std::string& _message, char _type = INFO) noexcept;

    const Message* getMessage(size_t n = 0) noexcept;

    size_t newMessagesAmount() noexcept;

}
