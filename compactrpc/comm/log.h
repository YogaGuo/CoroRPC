#ifndef COMPACTRPC_COMM_LOG_H
#define COMPACTRPC_COMM_LOG_H

#include <sstream>
#include <time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <memory>
#include <queue>
#include <vector>
#include <sys/types.h>
#include <semaphore.h>
#include "compactrpc/comm/Mutex.h"
#include "compactrpc/comm/config.h"

namespace compactrpc
{

    extern compactrpc::Config::ptr grpcConfig;
#define DebugLog                                                                                     \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::DEBUG >= compactrpc::gRpcConfig->m_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::DEBUG, __FILE__, __LINE__, __func__, compactprc::LogType::RPC_LOG))).getStringStream()

#define InfoLog                                                                                     \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::INFO >= compactrpc::gRpcConfig->m_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::INFO, __FILE__, __LINE__, __func__, compactrpc::LogType::RPC_LOG))).getStringStream()

#define WarnLog                                                                                     \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::WARN >= compactrpc::gRpcConfig->m_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::WARN, __FILE__, __LINE__, __func__, compactrpc::LogType::RPC_LOG))).getStringStream()

#define ErrorLog                                                                                     \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::ERROR >= compactrpc::gRpcConfig->m_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::ERROR, __FILE__, __LINE__, __func__, compactrpc::LogType::RPC_LOG))).getStringStream()

#define AppDebugLog                                                                                      \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::DEBUG >= compactrpc::gRpcConfig->m_app_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::DEBUG, __FILE__, __LINE__, __func__, compactrpc::LogType::APP_LOG))).getStringStream()

#define AppInfoLog                                                                                      \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::INFO >= compactrpc::gRpcConfig->m_app_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::INFO, __FILE__, __LINE__, __func__, compactrpc::LogType::APP_LOG))).getStringStream()

#define AppWarnLog                                                                                      \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::WARN >= compactrpc::gRpcConfig->m_app_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::WARN, __FILE__, __LINE__, __func__, compactrpc::LogType::APP_LOG))).getStringStream()

#define AppErrorLog                                                                                      \
    if (compactrpc::OpenLog() && compactrpc::LogLevel::ERROR >= compactrpc::gRpcConfig->m_app_log_level) \
    compactrpc::LogTmp(compactrpc::LogEvent::ptr(new compactrpc::LogEvent(compactrpc::LogLevel::ERROR, __FILE__, __LINE__, __func__, compactrpc::LogType::APP_LOG))).getStringStream()

    pid_t gettid();

    LogLevel stringToLevel(const std::string &str);

    std::string levelToString(LogLevel level);

    bool OpenLog();

    enum LogType
    {
        RPC_LOG = 1,
        APP_LOG = 2,
    };
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(LogLevel level, const char *file_name, int line, const char *func_bame, LogType type);

        ~LogEvent();

        std::stringstream &getStringStream();

        void log();

        void Exit(int code);

    private:
        timeval m_timeval;
        LogLevel m_level;
        pid_t m_pid{0};
        pid_t m_tid{0};
        int m_cor_is{0};

        const char *m_file_name;
        int m_line{0};
        const char *m_func_name;
        LogType m_type;
        std::string m_msg_no;
        std::string m_msg_no;
        std::stringstream m_ss;
    };

    class AsyncLogger
    {
    public:
        typedef std::shared_ptr<AsyncLogger> ptr;
        AsyncLogger(const char *file_name, const char *file_path, int max_size,
                    LogType logtype);

        ~AsyncLogger();

        void push(std::vector<std::string> &buffer);

        void flush();

        static void *excute(void *);

        void stop();

        std::queue<std::vector<std::string>> m_tasks;

    private:
        const char *m_file_name;
        const char *m_file_path;
        int m_max_size{0};
        LogType m_log_type;
        int m_no{0};
        bool m_need_reopen{false};
        FILE *m_file_handle{nullptr};
        std::string m_data;

        Mutex m_mutex;
        pthread_cond_t m_condition;

    public:
        pthread_t m_thread;
        sem_t m_semaphore;
    };

    class Logger
    {
    public:
        typedef std::shared_ptr<Logger> ptr;
        Logger();
        ~Logger();

        void init(const char *file_name, const char *file_path, int max_size, int sync_inteval);

        void log();

        void pushRpcLog(const std::string &log_msg);

        void pushAppLog(const std::string &msg);

        void loopFunc();

        void flush();

        void start();

        AsyncLogger::ptr getAsyncLogger()
        {
            return m_async_rpc_logger;
        }

        AsyncLogger::ptr getAsyncAppLogger()
        {
            return m_async_app_logger;
        }

        std::vector<std::string> m_buffer;
        std::vector<std::string> m_app_buffer;

    private:
        Mutex m_app_buff_mutex;
        Mutex m_buff_mutex;
        bool m_is_init{false};
        AsyncLogger::ptr m_async_rpc_logger;
        AsyncLogger::ptr m_async_app_logger;

        int m_sync_inteval{0};
    };
}
#endif
