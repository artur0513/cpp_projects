#include "Syslog.h"
#include <format>
#include <deque>
#include <fstream>
#include <cstdlib>

namespace Syslog {

    std::string typeToString(char _type) noexcept {
        switch (_type){
            case FATAL:
                return "FATAL";
            case ERROR:
                return "ERROR";
            case WARNING:
                return "WARNING";
            case INFO:
                return "INFO";
            case SUCSSES:
                return "SUCSSES";
            case DEBUG:
                return "DEBUG";
            default:
                return "UNKNOWN";
        }
    }

    std::string typeToColorCode(char _type) noexcept {
        switch (_type){
            case FATAL:
                return "\033[48;5;160m\033[38;5;0m";
            case ERROR:
                return "\033[48;5;0m\033[38;5;160m";
            case WARNING:
                return "\033[48;5;0m\033[38;5;220m";
            case INFO:
                return "\033[48;5;0m\033[38;5;252m";
            case SUCSSES:
                return "\033[48;5;0m\033[38;5;34m";
            case DEBUG:
                return "\033[48;5;0m\033[38;5;33m";
            default:
                return "\033[48;5;0m\033[38;5;252m";
        }
    }

    m3d::vec4f typeToColor(char _type) noexcept {
        switch (_type){
            case FATAL:
                return m3d::vec4f(0.5f, 0.0f, 0.0f, 1.0f);
            case ERROR:
                return m3d::vec4f(0.85f, 0.0f, 0.0f, 1.0f);
            case WARNING:
                return m3d::vec4f(0.9f, 0.9f, 0.0f, 1.0f);
            case INFO:
                return m3d::vec4f(1.0f, 1.0f, 1.0f, 1.0f);
            case SUCSSES:
                return m3d::vec4f(0.0f, 0.5f, 0.0f, 1.0f);
            case DEBUG:
                return m3d::vec4f(0.5f, 0.5f, 0.5f, 1.0f);
            default:
                return m3d::vec4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    void Syslog::Message::createSummary() noexcept {
        message.insert(0, 1, ' ');
        std::string temp = "[ " + std::format("{:%T}", std::chrono::floor<std::chrono::milliseconds>(time)) + ", " + typeToString(type) + " ]";
        summary = temp + message;
        summaryColors = typeToColorCode(type) + temp + typeToColorCode(INFO) + message;
    }


    Syslog::Message::Message(const std::string& _message, char _type){
        type = _type;
        color = typeToColor(type);
        message = _message;
        time = std::chrono::system_clock::now();
        createSummary();
    }

    const std::string& Syslog::Message::getSummary() const noexcept {
        return summary;
    }

    const std::string& Syslog::Message::getSummaryColors() const noexcept {
        return summaryColors;
    }

    const char Syslog::Message::getType() const noexcept {
        return type;
    }

    m3d::vec4f Syslog::Message::getColor() const noexcept{
        return color;
    }

    /* Works as private */
    namespace {
        size_t newMessage = 0;

        const unsigned maxMessgaes = 100;
        std::deque<Message> messages;
        const std::string fileOutName = "Syslog(" + std::string(__DATE__) + ").txt";
        std::ofstream fileOut;

        bool init() noexcept {
            fileOut.open(fileOutName);
            std::atexit([](){log("Syslog closing...", INFO); fileOut.close();});
            if (!fileOut.is_open())
                return 0;

            log("Syslog init sucssesful. Logs will be saved in " + fileOutName, SUCSSES);
            log("Build time: " + std::string(__DATE__) + " " + __TIME__, SUCSSES);
            log("Please note that message times are in UTC+0, and build time is in local time", WARNING);

            return 1;
        }
        static bool isInit = init();

    }

    void log(const std::string& _message, char _type) noexcept {
        if (messages.size() >= maxMessgaes)
            messages.pop_front();
        messages.emplace_back(_message, _type);

        Message* msg = &messages.back();
        std::cout << msg->getSummaryColors() << "\n";
        fileOut << msg->getSummary() << "\n";

        newMessage++;
    }

    const Message* getMessage(size_t n) noexcept {
        if (n >= maxMessgaes)
            return nullptr;
        return &messages[messages.size() - n - 1];
    }

    size_t newMessagesAmount() noexcept {
        auto temp = newMessage;
        newMessage = 0;
        return temp;
    }
}
