#pragma once

#if defined(__GNUC__) || defined(__clang__)
#define PRINTF_FMT(a, b) __attribute__((format(printf, a, b)))
#else
#define PRINTF_FMT(a, b)
#endif

typedef enum {
    LOG_DEBUG,
    LOG_WARN,
    LOG_ERROR,
} log_level_t;

void log_message(log_level_t level, const char *file, int line, const char *func, const char *fmt, ...)
    PRINTF_FMT(5, 6);

#define log_debug(fmt, ...) log_message(LOG_DEBUG, __FILE__, __LINE__, __func__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define log_warn(fmt, ...) log_message(LOG_WARN, __FILE__, __LINE__, __func__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define log_error(fmt, ...) log_message(LOG_ERROR, __FILE__, __LINE__, __func__, fmt __VA_OPT__(, ) __VA_ARGS__)
