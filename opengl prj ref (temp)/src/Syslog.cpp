#include "Syslog.h"
#include <format>
#include <deque>
#include <fstream>
#include <cstdlib>
#include <streambuf>

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

    class Message {
    private:
        char type;
        std::chrono::time_point<std::chrono::system_clock> time;
        std::string message;
        /* Probably add stacktrace support for fatal, error and debug messages */

        /* Summary is string that contains time, type and message together.
         * summaryColors is needed for terminals that support colors */
        std::string summary, summaryColors;

        void createSummary() noexcept {
            message.insert(0, 1, ' ');
            std::string temp = "[ " + std::format("{:%T}", std::chrono::floor<std::chrono::milliseconds>(time)) + ", " + typeToString(type) + " ]";
            summary = temp + message;
            summaryColors = typeToColorCode(type) + temp + typeToColorCode(INFO) + message;
        }

    public:
        Message(const std::string& _message, char _type = INFO){
            type = _type;
            message = _message;
            time = std::chrono::system_clock::now();
            createSummary();
        }

        const std::string& getSummary() noexcept {
            return summary;
        }

        const std::string& getSummaryColors() noexcept {
            return summaryColors;
        }
    };

    /* Works as private */
    namespace {
        const unsigned maxMessgaes = 30;
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
    }


}
