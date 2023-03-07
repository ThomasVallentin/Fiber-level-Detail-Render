#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <string>


class Logger {
public:
    template<typename... Args>
    inline void static debug(const char* format, const Args &... args) {
        fprintf(stdout, (std::string("DEBUG :   ") + format + "\n").c_str(), args...);
    }
    inline void static debug(const char* message) {
        fprintf(stdout, (std::string("DEBUG :   ") + message + "\n").c_str());
    }

    template<typename... Args>
    inline void static info(const char* format, const Args &... args) {
        fprintf(stdout, (std::string("INFO :    ") + format + "\n").c_str(), args...);
    }
    inline void static info(const char* message) {
        fprintf(stdout, (std::string("INFO :    ") + message + "\n").c_str());
    }

    template<typename... Args>
    inline void static warning(const char* format, const Args &... args) {
        fprintf(stderr, (std::string("WARNING : ") + format + "\n").c_str(), args...);
    }
    inline void static warning(const char* message) {
        fprintf(stderr, (std::string("WARNING : ") + message + "\n").c_str());
    }

    template<typename... Args>
    inline void static error(const char* format, const Args &... args) {
        fprintf(stderr, (std::string("ERROR :  ") + format + "\n").c_str(), args...);
    }
    inline void static error(const char* message) {
        fprintf(stderr, (std::string("ERROR :  ") + message + "\n").c_str());
    }

};

#ifdef ENABLE_DEBUG
#define LOG_DEBUG(...)   Logger::debug(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

#define LOG_INFO(...)    Logger::info(__VA_ARGS__)
#define LOG_WARNING(...) Logger::warning(__VA_ARGS__)
#define LOG_ERROR(...)   Logger::error(__VA_ARGS__)

#define ENABLE_ASSERTS

#ifdef ENABLE_ASSERTS
#define ASSERT(check, ...)    if (!(check)) { LOG_ERROR(__VA_ARGS__); }
#define ASSERT_OR_RETURN(check, returnValue, ...)    if (!(check)) { LOG_ERROR(__VA_ARGS__); return returnValue; }
#else
#define ASSERT(...)
#define ASSERT_OR_RETURN(...)
#endif

#endif  // LOGGING_H
