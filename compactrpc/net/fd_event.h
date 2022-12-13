/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-12 10:16:45
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-13 11:14:25
 */
#ifndef COMPACTRPC_FD_EVENT_H
#define COMPACTRPC_FD_EVENT_H

#include <functional>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <assert.h>
#include <vector>
#include "reactor.h"
#include "compactrpc/comm/log.h"
#include "compactrpc/coroutine/coroutine.h"
#include "compactrpc/comm/Mutex.h"

namespace compactrpc
{
    class Reactor;

    enum IOEvent
    {
        READ = EPOLLIN,
        WRITE = EPOLLOUT,
        ETMODEL = EPOLLET,
    };

    class FdEvent : public std::enable_shared_from_this<FdEvent>
    {
    public:
        typedef std::shared_ptr<FdEvent> ptr;

        FdEvent(compactrpc::Reactor *reactor, int fd = -1);

        FdEvent(int fd);

        virtual ~FdEvent();

        void handleEvent(int flag);

        void setCallback(IOEvent flag, std::function<void()> cb);

        std::function<void()> getCallback(IOEvent flag) const;

        void addListenEvent(IOEvent event);

        void delListenEvent(IOEvent event);

        void updateToReactor();

        void unregisterFromReactor();

        int getFd() const;

        void setFd(const int fd);

        int getListenEvent() const;

        Reactor *getReactor() const;

        void setReactor(Reactor *r);

        void setNonBlock();

        bool isNonBlock();

        void setCoroutine(Coroutine *cor);

        Coroutine *getCoroutine() const;

        void clearCoroutine();

    public:
        Mutex m_mutex;

    protected:
        int m_fd{-1};
        std::function<void()> m_read_callback;
        std::function<void()> m_write_callback;

        int m_listen_events{0};
        Reactor *m_reactor{nullptr};

        Coroutine *m_coroutine{nullptr};
    };

    class FdEventContianer
    {
    public:
        FdEventContianer(int size);

        FdEvent::ptr getFdEvent(int fd);

        static FdEventContianer *GetFdContianer();

    private:
        RWMutex m_mutex;
        std::vector<FdEvent::ptr> m_fds;
    };
}
#endif