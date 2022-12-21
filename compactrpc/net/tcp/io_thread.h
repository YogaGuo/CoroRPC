/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-20 17:49:09
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-21 12:57:42
 */
#ifndef COMPACTRPC_IO_THREAD_H
#define COMPACTRPC_IO_THREAD_H

#include <memory>
#include <map>
#include <atomic>
#include <sys/types.h>
#include <functional>
#include <semaphore.h>
#include <pthread.h>
#include "compactrpc/net/reactor.h"
#include "compactrpc/net/tcp/tcp_conn_time_wheel.h"
#include "compactrpc/coroutine/coroutine.h"

namespace compactrpc
{
    class TcpServer;

    class IOThread
    {

    public:
        typedef std::shared_ptr<IOThread> ptr;
        IOThread();

        ~IOThread();

        Reactor *getRector();

        void addClient();

        pthread_t getPthreadId();

        void setThreadIndex(const int index);

        int getThreadIndex();

        sem_t *getStartSemphore();

    public:
        static IOThread *getCurrentIOthread();

    private:
        static void *main(void *arg);
        Reactor *m_reactor{nullptr};
        pthread_t m_thread{0};
        pid_t m_tid{-1};
        TimerEvent::ptr m_timer_event{nullptr};
        int m_index{-1};

        sem_t m_init_semaphore;
        sem_t m_start_semaphore;
    };

    class IOThreadPool
    {
    public:
        typedef std::shared_ptr<IOThreadPool> ptr;

        IOThreadPool(int size);

        void start();

        IOThread *getIOThread();

        int getIOThreadPoolSize();

        void broadcastTask(std::function<void()> cb);

        void addTaskByIndex(int index, std::function<void()> cb);

        void addCoroutineToRandomThread(Coroutine::ptr cor, bool self = false);

        /**
         * @brief
         *
         * @param cb
         * @param self £º self = false, means random thread can't be current thread free cor,
         *                 or cause memory leak, call returnCoroutine(cor) to free cor
         * @return Coroutine::ptr
         */
        Coroutine::ptr addCoroutineToRandomThread(std::function<void()> cb, bool self = false);

        void addCoroutineToEachThread(std::function<void> cb);

        void addCoroutineToThreadByIndex(int index, std::function<void()> cb, bool self = false);

    private:
        int m_size{0};

        std::atomic<int> m_index{-1};

        std::vector<IOThread::ptr> m_io_threads;
    };
}
#endif