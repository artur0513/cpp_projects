#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>

namespace Syslog {

    enum : char {
        FATAL = 'f',
        ERROR = 'e',
        WARNING = 'w',
        INFO = 'i',
        SUCSSES = 's',
        DEBUG = 'd'
    };

    void log(const std::string& _message, char _type = INFO) noexcept;

}
