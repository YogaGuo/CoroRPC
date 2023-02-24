/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-12 10:15:59
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-21 21:56:44
 */
#ifndef COMPACTRPC_REACTOR_H
#define COMPACTRPC_REACTOR_H

#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <atomic>
#include <map>
#include <functional>
#include <queue>
#include <sys/eventfd.h>
#include "fd_event.h"
#include "timer.h"
#include "compactrpc/coroutine/coroutine.h"
#include "compactrpc/comm/Mutex.h"
#include <sys/epoll.h>
namespace compactrpc
{
    enum ReactorType
    {
        MainReactor = 1,
        SubReactor = 2
    };

    class FdEvent;
    class Timer;

    class Reactor
    {
    public:
        typedef std::shared_ptr<Reactor> ptr;

        explicit Reactor();

        ~Reactor();

        void addEvent(int fd, epoll_event event, bool is_wakeup = true);

        void delEvent(int fd, bool is_wakeup = true);

        void addTask(std::function<void()> task, bool is_wakeup = true);

        void addTask(std::vector<std::function<void()>> tasks, bool is_wakeup = true);

        void addCoroutine(compactrpc::Coroutine::ptr cor, bool is_wakeup = true);

        void wakeup();

        void loop();

        void stop();

        Timer *getTimer();

        pid_t getTid();

        void setReactorType(ReactorType type);

    public:
        static Reactor *GetReactor();

    private:
        void addWakeupFd();

        bool isLoopThread() const;

        void addEventInLoopThread(int fd, epoll_event event);

        void delEvenInLoopThread(int fd);

    private:
        int m_epfd{-1};

        int m_wake_fd{-1}; // »½ÐÑ

        int m_timer_fd{-1};

        bool m_stop_flag{false};

        bool m_is_looping{false};

        bool m_is_init_time{false};

        pid_t m_tid{0}; // thread id

        compactrpc::Mutex m_mutex;

        Timer *m_timer{nullptr};

        ReactorType m_reactor_type{SubReactor};

        std::vector<int> m_fds;

        std::atomic<int> m_fds_size;

        /**
         * @Yogaguo :    std::map<int, epoll_event> m_pending_add_fds
         *               1-----> add to epoll
         *               2------> del from epoll
         */
        std::map<int, epoll_event> m_pending_add_fds;

        std::vector<int> m_pending_del_fds;

        std::vector<std::function<void()>> m_pending_task;
    };

    class CoroutineTaskQueue
    {
    public:
        static CoroutineTaskQueue *GetCoroutineTaskQueue();

        void push(FdEvent *fd);

        FdEvent *pop();

    private:
        Mutex m_mutex;

        std::queue<FdEvent *> m_task;
    };
}
#endif