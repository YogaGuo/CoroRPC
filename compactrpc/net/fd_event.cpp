/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-12 10:15:59
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-22 17:22:54
 */
#include "fd_event.h"

namespace compactrpc
{
    static FdEventContianer *g_FdContainer{nullptr};

    FdEvent::FdEvent(compactrpc::Reactor *reactor, int fd) : m_fd(fd), m_reactor(reactor)
    {
        if (reactor == nullptr)
            ErrorLog << "create reactor first";
    }
    FdEvent::FdEvent(int fd) : m_fd(fd) {}

    FdEvent::~FdEvent() {}

    void FdEvent::handleEvent(int flag)
    {
        if (flag == READ)
        {
            m_read_callback();
        }
        else if (flag == WRITE)
        {
            m_write_callback();
        }
        else
            ErrorLog << "error flag";
    }

    void FdEvent::setCallback(IOEvent flag, std::function<void()> cb)
    {
        if (flag == READ)
        {
            m_read_callback = cb;
        }
        else if (flag == WRITE)
        {
            m_write_callback = cb;
        }
        else
            ErrorLog << "error flag";
    }

    std::function<void()> FdEvent::getCallback(IOEvent flag) const
    {
        if (flag == READ)
        {
            return m_read_callback;
        }
        else if (flag == WRITE)
        {
            return m_write_callback;
        }
        return nullptr;
    }

    void FdEvent::addListenEvent(IOEvent event)
    {
        if (m_listen_events & event)
        {
            ErrorLog << "already has this event, skip";
        }
        m_listen_events |= event;
        updateToReactor();
    }

    void FdEvent::delListenEvent(IOEvent event)
    {

        if (m_listen_events & event)
        {
            m_listen_events &= ~event;
            DebugLog << "delete succ";
            updateToReactor();
            return;
        }
        DebugLog << "this event not exit, skip";
    }

    void FdEvent::updateToReactor()
    {
        epoll_event event;
        event.events = m_listen_events;
        event.data.ptr = this;
        if (m_reactor == nullptr)
            m_reactor = compactrpc::Reactor::GetReactor();
        m_reactor->addEvent(m_fd, event);
    }

    void FdEvent::unregisterFromReactor()
    {
        if (!m_reactor)
            m_reactor = compactrpc::Reactor::GetReactor();
        m_reactor->delEvent(m_fd);
        m_listen_events = 0;
        m_read_callback = nullptr;
        m_write_callback = nullptr;
    }

    int FdEvent::getFd() const
    {
        return m_fd;
    }

    void FdEvent::setFd(int fd)
    {
        m_fd = fd;
    }

    int FdEvent::getListenEvent() const
    {
        return m_listen_events;
    }

    Reactor *FdEvent::getReactor() const
    {
        return m_reactor;
    }

    void FdEvent::setReactor(Reactor *r)
    {
        m_reactor = r;
    }

    void FdEvent::setNonBlock()
    {
        if (m_fd == -1)
        {
            ErrorLog << "error fd == -1";
            return;
        }

        int flag = fcntl(m_fd, F_GETFL, 0);
        if (flag & O_NONBLOCK)
        {
            DebugLog << "m_fd already set nonblock";
            return;
        }
        fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
        flag = fcntl(m_fd, F_GETFL, 0);
        if (flag & O_NONBLOCK)
            DebugLog << "set nonblock suss";
        else
            ErrorLog << "set nonblock error";
    }

    bool FdEvent::isNonBlock()
    {
        if (m_fd == -1)
        {
            ErrorLog << "error m_fd == -1";
            return false;
        }
        int flag = fcntl(m_fd, F_GETFL, 0);
        return (flag & O_NONBLOCK);
    }

    void FdEvent::setCoroutine(Coroutine *cor)
    {
        m_coroutine = cor;
    }

    Coroutine *FdEvent::getCoroutine() const
    {
        return m_coroutine;
    }

    void FdEvent::clearCoroutine()
    {
        m_coroutine = nullptr;
    }

    FdEventContianer::FdEventContianer(int size)
    {
        for (int i = 0; i < size; i++)
        {
            m_fds.push_back(std::make_shared<FdEvent>(i));
        }
    }

    FdEventContianer *FdEventContianer::GetFdContianer()
    {
        if (g_FdContainer == nullptr)
            g_FdContainer = new FdEventContianer(1000);
        return g_FdContainer;
    }

    FdEvent::ptr FdEventContianer::getFdEvent(int fd)
    {
        RWMutex::ReadLock rlock(m_mutex);
        if (fd < static_cast<int>(m_fds.size()))
        {
            FdEvent::ptr p = m_fds[fd];
            rlock.unlock();
            return p;
        }
        rlock.unlock();

        RWMutex::WriteLock wlock(m_mutex);
        int n = static_cast<int>(n * 1.5);
        for (int i = m_fds.size(); i < n; i++)
        {
            m_fds.push_back(std::make_shared<FdEvent>(i));
        }
        FdEvent::ptr p = m_fds[fd];
        wlock.unlock();
        return p;
    }
}