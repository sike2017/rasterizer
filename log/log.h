#pragma once
#define USE_VISUAL_STUDIO_OUTPUT
#include <memory>
#include <string>
#include <Windows.h>
#include <atlstr.h>

class LogEndl {
public:
    LogEndl() {}
    ~LogEndl() {}

    char character = '\n';
};

class Log
{
public:
    Log() {}
    ~Log() {}

    void print(const char* strOutputString, ...);
    Log& operator<<(const char* str);
    Log& operator<<(const std::string& str);
    Log& operator<<(float e);
    Log& operator<<(LogEndl le);
};

extern Log rlog;
extern LogEndl rendl;
