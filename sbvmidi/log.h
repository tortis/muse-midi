/****************************************************************************
* Copyright (C) 2016 Autotheory
* All rights reserved
*
****************************************************************************/

#ifndef _LOG_H_
#define _LOG_H_

#define DEBUG  1
#define INFO   2
#define WARN   3
#define ERROR  4

#define LOG_THRESHOLD (DEBUG)

#define PRINT_LOG(...) \
    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "%s:%d ", __FUNCTION__, __LINE__);\
    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__);\
    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "\n"); \

#if (LOG_THRESHOLD <= ERROR)
#define LOG_ERROR(...) \
    PRINT_LOG(__VA_ARGS__)
#else
#define LOG_ERROR(...)
#endif

#if (LOG_THRESHOLD <= WARN)
#define LOG_WARN(...) \
    PRINT_LOG(__VA_ARGS__)
#else
#define LOG_WARN(...)
#endif

#if (LOG_THRESHOLD <= INFO)
#define LOG_INFO(...) \
    PRINT_LOG(__VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

#if (LOG_THRESHOLD <= DEBUG)
#define LOG_DEBUG(...) \
    PRINT_LOG(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

#if DBG==1
#define LOG(Level, ...) \
    LOG_##Level(__VA_ARGS__)
#else
#define LOG(Level, ...)
#endif

#endif // _LOG_H_