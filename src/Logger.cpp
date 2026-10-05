#include "Logger.h"
#include <stdarg.h>

Logger logger;

Logger::Logger() : minLogLevel(INFO) {}

void Logger::begin(LogLevel minLevel) {
    minLogLevel = minLevel;
    logBuffer.clear();
    Serial.println("[Logger] Initialized");
}

void Logger::log(LogLevel level, const char* component, const char* message) {
    if (level < minLogLevel) {
        return;
    }
    
    unsigned long timestamp = millis();
    
    Serial.printf("[%lu] [%s] [%s] %s\n", 
                 timestamp, 
                 getLevelString(level), 
                 component, 
                 message);
    
    addToBuffer(level, component, message);
}

void Logger::logf(LogLevel level, const char* component, const char* format, ...) {
    if (level < minLogLevel) {
        return;
    }
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    log(level, component, buffer);
}

void Logger::debug(const char* component, const char* message) {
    log(DEBUG, component, message);
}

void Logger::info(const char* component, const char* message) {
    log(INFO, component, message);
}

void Logger::warning(const char* component, const char* message) {
    log(WARNING, component, message);
}

void Logger::error(const char* component, const char* message) {
    log(ERROR, component, message);
}

void Logger::critical(const char* component, const char* message) {
    log(CRITICAL, component, message);
}

void Logger::setLogLevel(LogLevel level) {
    minLogLevel = level;
}

LogLevel Logger::getLogLevel() const {
    return minLogLevel;
}

std::vector<LogEntry> Logger::getRecentLogs(int count) {
    int start = logBuffer.size() > count ? logBuffer.size() - count : 0;
    return std::vector<LogEntry>(logBuffer.begin() + start, logBuffer.end());
}

void Logger::clearLogs() {
    logBuffer.clear();
}

const char* Logger::getLevelString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARNING: return "WARN";
        case ERROR: return "ERROR";
        case CRITICAL: return "CRIT";
        default: return "UNKNOWN";
    }
}

void Logger::addToBuffer(LogLevel level, const char* component, const char* message) {
    LogEntry entry;
    entry.timestamp = millis();
    entry.level = level;
    entry.component = String(component);
    entry.message = String(message);
    
    logBuffer.push_back(entry);
    
    if (logBuffer.size() > MAX_LOG_ENTRIES) {
        logBuffer.erase(logBuffer.begin());
    }
}
