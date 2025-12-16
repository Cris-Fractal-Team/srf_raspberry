#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
   public:
    static void init(const std::string& fileName = "",
                     LogLevel minimumLevel = LogLevel::DEBUG,
                     bool enableConsoleOutput = true) {
        instance().initializeLogger(fileName, minimumLevel,
                                    enableConsoleOutput);
    }

    static Logger& instance() {
        static Logger loggerInstance;
        return loggerInstance;
    }

    void log(LogLevel level, const std::string& componentName,
             const std::string& message) {
        if (level < minimumLogLevel) return;

        std::lock_guard<std::mutex> lock(logMutex);

        std::string timestamp = getCurrentTimestamp();
        std::string levelText = convertLevelToString(level);

        std::ostringstream stream;
        stream << "[" << timestamp << "] " << levelText << " "
               << "[" << componentName << "] " << message << "\n";

        const std::string logEntry = stream.str();

        if (consoleOutputEnabled) {
            std::cout << logEntry;
        }

        if (logFileStream.is_open()) {
            logFileStream << logEntry;
            logFileStream.flush();
        }
    }

   private:
    std::ofstream logFileStream;
    std::mutex logMutex;
    LogLevel minimumLogLevel = LogLevel::DEBUG;
    bool consoleOutputEnabled = true;
    bool loggerInitialized = false;

    Logger() = default;

    ~Logger() {
        if (logFileStream.is_open()) {
            logFileStream.close();
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void initializeLogger(const std::string& fileName, LogLevel minimumLevel,
                          bool enableConsoleOutput) {
        std::lock_guard<std::mutex> lock(logMutex);

        if (loggerInitialized) {
            return;
        }

        if (!fileName.empty()) {
            logFileStream.open(fileName, std::ios::app);
            if (!logFileStream.is_open()) {
                std::cerr << "[LOGGER] No se pudo abrir el archivo de log: "
                          << fileName << std::endl;
            }
        }

        minimumLogLevel = minimumLevel;
        consoleOutputEnabled = enableConsoleOutput;
        loggerInitialized = true;
    }

    static std::string getCurrentTimestamp() {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto timeValue = system_clock::to_time_t(now);
        auto millisecondsPart =
            duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm localTime{};
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&localTime, &timeValue);
#else
        localtime_r(&timeValue, &localTime);
#endif

        std::ostringstream stream;
        stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << '.'
               << std::setw(3) << std::setfill('0') << millisecondsPart.count();
        return stream.str();
    }

    static std::string convertLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:
                return "DEBUG   ";
            case LogLevel::INFO:
                return "INFO    ";
            case LogLevel::WARNING:
                return "WARNING ";
            case LogLevel::ERROR:
                return "ERROR   ";
            case LogLevel::CRITICAL:
                return "CRITICAL";
            default:
                return "UNKNOWN ";
        }
    }
};

#define LOG_INTERNAL(level, component, msg)                          \
    do {                                                             \
        std::ostringstream _log_stream;                              \
        _log_stream << msg;                                          \
        Logger::instance().log(level, component, _log_stream.str()); \
    } while (false)

#define LOG_DEBUG(component, msg) LOG_INTERNAL(LogLevel::DEBUG, component, msg)
#define LOG_INFO(component, msg) LOG_INTERNAL(LogLevel::INFO, component, msg)
#define LOG_WARN(component, msg) LOG_INTERNAL(LogLevel::WARNING, component, msg)
#define LOG_ERROR(component, msg) LOG_INTERNAL(LogLevel::ERROR, component, msg)
#define LOG_CRITICAL(component, msg) \
    LOG_INTERNAL(LogLevel::CRITICAL, component, msg)
