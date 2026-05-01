#ifndef __LOG_H
#define __LOG_H

#define USE_LOG		1

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} log_level_t;

// 使用宏简化调用
#define LOG_DEBUG(tag, ...) log_msg_level(LOG_LEVEL_DEBUG, tag, __VA_ARGS__)
#define LOG_INFO(tag, ...)  log_msg_level(LOG_LEVEL_INFO, tag, __VA_ARGS__)
#define LOG_WARN(tag, ...)  log_msg_level(LOG_LEVEL_WARN, tag, __VA_ARGS__)
#define LOG_ERROR(tag, ...) log_msg_level(LOG_LEVEL_ERROR, tag, __VA_ARGS__)

#endif