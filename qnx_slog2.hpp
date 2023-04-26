#pragma once
#include <stdio.h>
#include <process.h>
#include <sys/slog2.h>
#include <fcntl.h>
#include <mutex>
extern char *__progname;
enum LOG_TYPE {
    LOG_TYPE_NONE,
    LOG_TYPE_PRINTF,
    LOG_TYPE_QNX
};
enum LOG_LEVEL {
    LOG_ERROR,         // level is highest
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
};
#define LOG_TAG "qnx_slog2"
class qnx_slog2 {
private:
    short log_type_ = LOG_TYPE_QNX;
    short log_level_ = LOG_DEBUG;
    int log_page_num_ = 4;
    int log_id_ = 116;
    std::mutex lock_;
    char print_buffer_[1024] = { 0 };
private:
    qnx_slog2() = default;
    virtual ~qnx_slog2() {
        uninit();
    }
private:
    inline bool log_block(short level) {
        return level > log_level_;
    }
    inline void log_print(short level) {
        switch(level)
        {
        case LOG_ERROR:
            // red
            printf("\033[1m\033[40;31m [E] %s \033[0m\n", print_buffer_);
            break;
        case LOG_WARN:
            // yellow
            printf("\033[1m\033[40;33m [W] %s \033[0m\n", print_buffer_);
            break;
        case LOG_INFO:
            // white
            printf("\033[1m\033[40;37m [I] %s \033[0m\n", print_buffer_);
            break;
        case LOG_DEBUG:
            // green
            printf("\033[1m\033[40;36m [D] %s \033[0m\n", print_buffer_);
            break;
        }
    }
    inline short match_level(short level) {
        short log_level = SLOG2_ERROR;
        switch(level)
        {
        case LOG_ERROR:
            log_level = SLOG2_ERROR;
            break;
        case LOG_WARN:
            log_level = SLOG2_WARNING;
            break;
        case LOG_INFO:
            log_level = SLOG2_INFO;
            break;
        case LOG_DEBUG:
            log_level = SLOG2_DEBUG1;
            break;
        }
        return log_level;   
    }
public:
    inline static qnx_slog2& get_log() {
        static qnx_slog2 slog2_instance;
        return slog2_instance;
    }
    inline void set_log_type(short type) {
        log_type_ = type;
    }
    inline void set_log_level(short level) {
        log_level_ = level;
    }
    inline void set_log_page_num(int num) {
        log_page_num_ = num;
    }
    inline void set_log_id(int id) {
        log_id_ = id;
    }
    bool init() {
        if (log_type_ != LOG_TYPE_QNX) {
            return true;
        }
        std::unique_lock<std::mutex>lock(lock_);
        slog2_buffer_set_config_t config = { 0 };
        slog2_buffer_t buf = { 0 };
        config.buffer_set_name = __progname;
        config.num_buffers = 1;                  //  log buffer集 中有多少个buffer
        config.verbosity_level = SLOG2_DEBUG1;   //  整个log buffer集中的最小verbosity_level，只有高于verbosity_level的log才会写入log buffer中
        config.buffer_config[0].buffer_name = LOG_TAG;
        config.buffer_config[0].num_pages = log_page_num_;      // 用于定于log buffer的大小，一个page是4kB
        if (0 == slog2_register(&config, &buf, SLOG2_DISCARD_NEWLINE)) {
            slog2_set_default_buffer(buf);
            return true;
        }
        return false;
    }
    void uninit() {
        if (log_type_ != LOG_TYPE_QNX) {
            return;
        }
        std::unique_lock<std::mutex>lock(lock_);
        slog2_reset();
    }
    void log_output(short level, const char *fmt, ...) {
        if (true == log_block(level)) {
            return;
        }
        std::unique_lock<std::mutex>lock(lock_);
        va_list args;
        va_start(args, fmt);
        switch(log_type_) {
            case LOG_TYPE_QNX:
                vslog2f(nullptr, log_id_, match_level(level), fmt, args);
                break;
            case LOG_TYPE_PRINTF:
            {
                memset(print_buffer_, 0, sizeof(print_buffer_));
                vsnprintf(print_buffer_, sizeof(print_buffer_), fmt, args);
                log_print(level);
                break;
            }
        }
        va_end(args);
    }
};
#define SLOG_INIT()          qnx_slog2::get_log().init()
#define SLOG_E(fmt, ...)     qnx_slog2::get_log().log_output(LOG_ERROR, "[" LOG_TAG "] [E] %s:%d %s() " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define SLOG_W(fmt, ...)     qnx_slog2::get_log().log_output(LOG_WARN, "[" LOG_TAG "] [W] %s:%d %s() " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define SLOG_I(fmt, ...)     qnx_slog2::get_log().log_output(LOG_INFO, "[" LOG_TAG "] [I] %s:%d %s() " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define SLOG_D(fmt, ...)     qnx_slog2::get_log().log_output(LOG_DEBUG, "[" LOG_TAG "] [D] %s:%d %s() " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)