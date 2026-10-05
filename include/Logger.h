#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <vector>

enum LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

struct LogEntry {
    unsigned long timestamp;
    LogLevel level;
    String component;
    String message;
};

class Logger {
public:
    Logger();
    
    void begin(LogLevel minLevel = INFO);
    void log(LogLevel level, const char* component, const char* message);
    void logf(LogLevel level, const char* component, const char* format, ...);
    
    void debug(const char* component, const char* message);
    void info(const char* component, const char* message);
    void warning(const char* component, const char* message);
    void error(const char* component, const char* message);
    void critical(const char* component, const char* message);
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;
    
    std::vector<LogEntry> getRecentLogs(int count = 10);
    void clearLogs();
    
private:
    LogLevel minLogLevel;
    std::vector<LogEntry> logBuffer;
    const int MAX_LOG_ENTRIES = 100;
    
    const char* getLevelString(LogLevel level);
    void addToBuffer(LogLevel level, const char* component, const char* message);
};

extern Logger logger;

#endif
