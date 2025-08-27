#include "util/log.h"

#include <stdarg.h>
#include <stdio.h>

static const char *log_level_str(log_level_t level) {
    switch (level) {
    case LOG_DEBUG:
        return "DEBUG";
    case LOG_WARN:
        return "WARN";
    case LOG_ERROR:
        return "ERROR";
    default:
        return "";
    }
}

void log_message(log_level_t level, const char *file, int line, const char *func, const char *fmt, ...) {
    FILE *out = stderr;

    if (level == LOG_DEBUG) {
        out = stdout;
        fprintf(out, "[%s] ", log_level_str(level));
    } else {
        fprintf(out, "[%s] %s:%d:%s(): ", log_level_str(level), file, line, func);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
    fputc('\n', out);
    fflush(out);
}
