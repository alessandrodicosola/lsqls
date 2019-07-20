#ifndef __ASSERT__HPP__
#define __ASSERT__HPP__

#define ASSERT(Pred, Msg) _assert(#Pred, Pred, __FILE__, __LINE__, Msg)
#define ASSERT_NOMSG(Pred) ASSERT(Pred, "")

#include <iostream>

void _assert(const char *expr, bool pred, const char *file, const int line, const char *msg)
{
    if (!pred)
    {
        std::cerr << file << ":"
                  << line << ":"
                  << "Assertion `" << expr << "` failed: "
                  << msg << "\n";
        exit(1);
    }
}

void _assert(const char *expr, bool pred, const char *file, const int line, const std::string& msg)
{
    _assert(expr, pred, file, line,msg.c_str());
}
void _assert(const char *expr, bool pred, const char *file, const int line, const uint64_t msg)
{
    std::string _msg = std::to_string(msg);
    _assert(expr, pred, file, line, _msg.c_str());
}
void _assert(const char *expr, bool pred, const char *file, const int line, const bool msg)
{
    _assert(expr, pred, file, line, msg ? "TRUE" : "FALSE");
}

#endif