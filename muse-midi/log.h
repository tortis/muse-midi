/*
  ==============================================================================
   This file is part of the Springbeats Virtual MIDI Driver (sbvmidi).
   Copyright (c) 2016 - Springbeats SARL.
   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3
   Details of these licenses can be found at: www.gnu.org/licenses
   Sbvmidi is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
   ------------------------------------------------------------------------------
   To release a closed-source product which uses sbvmidi, commercial licenses are
   available: visit www.springbeats.com for more information.
  ==============================================================================
*/
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
