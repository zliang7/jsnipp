/*
 * Copyright 2016 Alibaba Group Holding Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#ifndef YUNOS_BASE_INCLUDE_LOG_LOG_H
#define YUNOS_BASE_INCLUDE_LOG_LOG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

//#include "util/Util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define YUNOS_CONDITION(cond) (__builtin_expect((cond)!=0, 0))

#ifndef LOG_V
#ifdef _BUILD_TYPE_eng
#define LOG_V(...) ((void)YUNOS_LOG(Verbose, LOG_TAG, __VA_ARGS__))
#else
#define LOG_V(...) (void)0
#endif
#endif

#ifndef if_LOG_V
#define if_LOG_V(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_LOG(Verbose, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef LOG_D
#ifdef _BUILD_TYPE_eng
#define LOG_D(...) ((void)YUNOS_LOG(Debug, LOG_TAG, __VA_ARGS__))
#else
#define LOG_D(...) (void)0
#endif
#endif

#ifndef if_LOG_D
#define if_LOG_D(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_LOG(Debug, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef LOG_I
#define LOG_I(...) ((void)YUNOS_LOG(Info, LOG_TAG, __VA_ARGS__))
#endif

#ifndef if_log_i
#define if_log_i(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_LOG(Info, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef LOG_W
#define LOG_W(...) ((void)YUNOS_LOG(Warn, LOG_TAG, __VA_ARGS__))
#endif

#ifndef if_LOG_W
#define if_LOG_W(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_LOG(Warn, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef LOG_E
#define LOG_E(...) ((void)YUNOS_LOG(Error, LOG_TAG, __VA_ARGS__))
#endif

#ifndef if_LOG_E
#define if_LOG_E(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_LOG(Error, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

/**
 * Trace APIs is for adding tracing log of the system with defined format.
 * Critical: will be logged as L0.
 * Brief:    will be logged as L1.
 * Detail:   will be logged as L2.
 */
#ifndef TRACE_CRITICAL
#define TRACE_CRITICAL(category, module, ...) \
    ((void)YUNOS_LOG(Info, "TraceInfo-" category "-" module "-L0", __VA_ARGS__))
#endif

#ifndef TRACE_BRIEF
#define TRACE_BRIEF(category, module, ...) \
    ((void)YUNOS_LOG(Info, "TraceInfo-" category "-" module "-L1", __VA_ARGS__))
#endif

#ifndef TRACE_DETAIL
#define TRACE_DETAIL(category, module, ...) \
    ((void)YUNOS_LOG(Info, "TraceInfo-" category "-" module "-L2", __VA_ARGS__))
#endif

#ifndef YUNOS_LOG
#define YUNOS_LOG(priority, tag, ...) \
    yunosLogPrint(kLogIdMain, kLogPriority##priority, tag, __VA_ARGS__)
#endif

#ifndef YUNOS_SLOG
#define YUNOS_SLOG(priority, tag, ...) \
    yunosLogPrint(kLogIdSystem, kLogPriority##priority, tag, __VA_ARGS__)
#endif

#ifndef SLOG_V
#ifdef _BUILD_TYPE_eng
#define SLOG_V(...) ((void)YUNOS_SLOG(Verbose, LOG_TAG, __VA_ARGS__))
#else
#define SLOG_V(...) (void)0
#endif
#endif

#ifndef if_SLOG_V
#define if_SLOG_V(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_SLOG(Verbose, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef SLOG_D
#ifdef _BUILD_TYPE_eng
#define SLOG_D(...) ((void)YUNOS_SLOG(Debug, LOG_TAG, __VA_ARGS__))
#else
#define SLOG_D(...) (void)0
#endif
#endif

#ifndef if_SLOG_D
#define if_SLOG_D(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_SLOG(Debug, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef SLOG_I
#define SLOG_I(...) ((void)YUNOS_SLOG(Info, LOG_TAG, __VA_ARGS__))
#endif

#ifndef if_SLOG_I
#define if_SLOG_I(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_SLOG(Info, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef SLOG_W
#define SLOG_W(...) ((void)YUNOS_SLOG(Warn, LOG_TAG, __VA_ARGS__))
#endif

#ifndef if_SLOG_W
#define if_SLOG_W(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_SLOG(Warn, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef SLOG_E
#define SLOG_E(...) ((void)YUNOS_SLOG(Error, LOG_TAG, __VA_ARGS__))
#endif

#ifndef if_SLOG_E
#define if_SLOG_E(cond, ...) \
    ( (YUNOS_CONDITION(cond)) \
    ? ((void)YUNOS_SLOG(Error, LOG_TAG, __VA_ARGS__)) \
    : (void)0 )
#endif

#ifndef LOG_T
#define LOG_T(id, msg) yunosLogPrint(kLogIdMain, kLogPriorityInfo, LOG_TAG, (msg)); \
    yunosTraceEvent((id), (msg))
#endif

/**
 * Represents the logging level.
 */
enum {
    kLogPriorityUnknown = 0,
    kLogPriorityDefault,
    kLogPriorityVerbose,
    kLogPriorityDebug,
    kLogPriorityInfo,
    kLogPriorityWarn,
    kLogPriorityError,
    kLogPriorityFatal,
    kLogPrioritySilent
};

enum {
    kLogIdMin = 0,

    kLogIdMain = 0,
    kLogIdRadio = 1,
    kLogIdEvents = 2,
    kLogIdSystem = 3,
    kLogIdCrash = 4,

    kLogIdMax
};

YUNOS_EXPORT int yunosLogWrite(int id, int prio, const char *tag, const char *text);

YUNOS_EXPORT int yunosLogPrint(int id, int prio, const char *tag, const char *fmt, ...)
        __attribute__((__format__(printf, 4, 5)));

YUNOS_EXPORT int yunosLogVPrint(int id, int prio, const char *tag, const char *fmt, va_list ap);

YUNOS_EXPORT int yunosGetLogLevel();

YUNOS_EXPORT int yunosTraceEvent(int eventId, const char* data);

YUNOS_EXPORT int yunosEventWrite(int32_t tag, const char* payload, size_t len);
YUNOS_EXPORT int yunosEventWriteInt(int32_t tag, int32_t value);
YUNOS_EXPORT int yunosEventWriteLong(int32_t tag, int64_t value);
YUNOS_EXPORT int yunosEventWriteStr(int32_t tag, const char* value);

#ifdef __cplusplus
}
#endif

#endif /* YUNOS_BASE_INCLUDE_LOG_LOG_H */
