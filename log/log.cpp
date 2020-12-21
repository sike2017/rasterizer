#include "log.h"

#ifdef USE_VISUAL_STUDIO_OUTPUT
void Log::print(const char* fmt, ...)
{
    char strBuffer[4096] = { 0 };

    va_list vlArgs;
    va_start(vlArgs, fmt);
    _vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, fmt, vlArgs);

    va_end(vlArgs);
    OutputDebugString(CA2W(strBuffer));
}
#else
void Log::print(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}
#endif
Log& Log::operator<<(const char* str) {
    print("%s", str);
    return *this;
}
Log& Log::operator<<(const std::string& str)
{
    print("%s", str.c_str());
    return *this;
}
Log& Log::operator<<(float e) {
    print("%f", e);
    return *this;
}
Log& Log::operator<<(LogEndl le) {
    print("%c", le.character);
    return *this;
}

Log rlog;
LogEndl rendl; // 实例表示换行符