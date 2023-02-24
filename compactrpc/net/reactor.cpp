/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-12 10:15:59
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-22 17:17:54
 */
#include "compactrpc/net/reactor.h"
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "compactrpc/comm/log.h"
#include "compactrpc/coroutine/coroutine.h"
extern read_fun_ptr_t g_sys_read_fun; // sys_call read
extern write_fun_ptr_t g_sys_write_fun;

namespace compactrpc
{
    static thread_local Reactor *t_reactor_ptr = nullptr;

    static thread_local int t_max_epoll_timeout = 10000; // ms

    static CoroutineTaskQueue *t_coroutine_task_queue = nullptr;

    Reactor::Reactor()
    {
        /**
         * @ 一个线程只能有一个reactor
         *
         */
        if (t_reactor_ptr != nullptr)
        {
            ErrorLog << "this thread has already created a reactor";
            Exit(0);
        }

        m_tid = getpid();

        DebugLog << "thread[" << m_tid << "] succ create a reaactor obj";
        t_reactor_ptr = this;

        if (m_epfd == epoll_create(1) <= 0)
        {
            ErrorLog << "start server error, epoll_create() error, sys error =" << strerror(errno);
            Exit(0);
        }
        else
        {
            DebugLog << "m_epfd ==" << m_epfd;
        }

        if ((m_wake_fd = eventfd(0, EFD_NONBLOCK)) <= 0)
        {
            ErrorLog << "start server error, eventfd() error, sys_error =" << strerror(errno);
            Exit(0);
        }
        else
        {
            DebugLog << "m_wake_fd ==" << m_wake_fd;
        }
        addWakeupFd();
    }

    Reactor::~Reactor()
    {
        DebugLog << "~Ractor";
        close(m_epfd);
        if (m_timer != nullptr)
        {
            delete m_timer;
            m_timer = nullptr;
        }
        t_reactor_ptr = nullptr;
    }

    Reactor *Reactor::GetReactor()
    {
        if (t_reactor_ptr == nullptr)
        {
            DebugLog << "create a new reator";
            t_reactor_ptr = new Reactor();
        }
        return t_reactor_ptr;
    }

    void Reactor::addEvent(int fd, epoll_event event, bool is_wakeup /*true*/)
    {
        if (fd == -1)
        {
            ErrorLog << "add error. because fd is invalid, fd == -1";
            return;
        }

        if (isLoopThread())
        {
            addEventInLoopThread(fd, event);
            return;
        }
        /**
         * @brief if it called other thread, need lock,
         *       example: 线程A往线程B加入一个fd, 线程A实际上只是把这个fd加入一个pending队列里面
         *      然后线程B在某个时刻会去捞取pending队列里面的fd, 然后添加到自己的epoll中，这样
         *       就避免了加锁, 而如果是线程B自己往自己里面添加fd, 直接添加就行
         */
        Mutex::Lock lock(m_mutex);
        m_pending_add_fds.insert({fd, event});

        if (is_wakeup)
            wakeup();
    }

    void Reactor::delEvent(int fd, bool is_wakeup /*true*/)
    {
        if (fd == -1)
        {
            ErrorLog << "add error, fd invalid, fd = -1";
            return;
        }

        if (isLoopThread())
        {
            delEventLoopInThread(fd);
            return;
        }

        Mutex::Lock lock(m_mutex);
        m_pending_del_fds.push_back(fd);
        if (is_wakeup)
            wakeup();
    }

    void Reactor::wakeup()
    {
        if (!m_is_looping)
            return;

        uint64_t tmp = 1;
        uint64_t *p = &tmp;
        if (g_sys_write_fun(m_wake_fd, p, 8) != 8)
        {
            ErrorLog << "write wakeupfd[" << m_wake_fd << "] error";
            return;
        }
    }

    bool Reactor::isLoopThread() const
    {
        if (m_tid == gettid())
            return true;
        return false;
    }

    void Reactor::addWakeupFd()
    {
        int op = EPOLL_CTL_ADD;
        epoll_event event;
        event.data.fd = m_wake_fd;
        event.events = EPOLLIN;
        if (epoll_ctl(m_epfd, op, m_wake_fd, event) != 0)
        {
            ErrorLog << "epoll_ctl error, fd[" << m_wake_fd << "], errno=" << strerror(errno);
            return;
        }
        m_fds.push_back(m_wake_fd);
    }

    // don't need lock, only this thread call
    void Reactor::addEventInLoopThread(int fd, epoll_event event)
    {
        assert(isLoopThread());

        int op = EPOLL_CTL_ADD;
        bool is_add = true;
        auto it = std::find(m_fds.begin(), m_fds.end(), fd);
        if (it != m_fds.end())
        {
            is_add = false;
            op = EPOLL_CTL_MOD;
        }
        if (epoll_ctl(m_epfd, op, fd, &event) != 0)
        {
            ErrorLog << "epoll_ctl error, fd[" << fd << "], errno=" << strerror(errno);
            return;
        }
        if (is_add)
        {
            m_fds.push_back(fd);
        }
        DebugLog << "epoll_ctl add succ, fd[" << fd << "]";
    }

    void Reactor::delEventLoopInThread(int fd)
    {
        assert(isLoopThread());

        auto it = std::find(m_fds.begin(), m_fds.end(), fd);
        if (it == m_fds.end())
        {
            DebugLog << "fd[" << fd << "] is not in this loop";
            return;
        }
        int op = EPOLL_CTL_DEL;
        if (epoll_ctl(m_epfd, op, fd, nullptr) != 0)
        {
            ErrorLog << "epoll_ctl error, fd[" << fd << "], errno=" << strerror(errno);
            return;
        }
        m_fds.erase(it);
        DebugLog << "del succ fd[" << fd << "]";
    }

    void Reactor::loop()
    {
        assert(isLoopThread());

        if (m_is_looping)
        {
            return;
        }

        m_is_looping = true;
        m_stop_flag = false;

        Coroutine *first_cor = nullptr;

        while (!m_stop_flag)
        {
            const int MAX_EVENT = 10;

            epoll_event re_event[MAX_EVENT + 1];

            if (first_cor)
            {
                compactrpc::Coroutine::Resume(first_cor);
                first_cor = nullptr;
            }

            if (m_reactor_type != MainReactor)
            {
                FdEvent *ptr = nullptr;

                while (1)
                {
                    ptr = CoroutineTaskQueue::GetCoroutineTaskQueue()->pop();
                    if (ptr)
                    {
                        ptr->setReactor(this);
                        compactrpc::Coroutine::Resume(ptr->getCoroutine());
                    }
                    else
                    {
                        break;
                    }
                }
            }
            Mutex::Lock lock(m_mutex);
            std::vector<std::function<void()>> tmp_tasks;
            tmp_tasks.swap(m_pending_task);
            lock.unlock();

            for (decltype(tmp_tasks.size()) i = 0; i < tmp_tasks.size(); i++)
            {
                DebugLog << "begin to excute task[" << i << "]";
                if (tmp_tasks[i])
                {
                    tmp_tasks[i]();
                }
                DebugLog << "end excute tasks[" << i << "]";
            }

            int rt = epoll_wait(m_epfd, re_event, MAX_EVENT, t_max_epoll_timeout);
            if (rt < 0)
            {
                ErrorLog << "epoll_wait error, skip, errno=" << strerror(errno);
            }
            else
            {
                for (int i = 0; i < rt; i++)
                {
                    epoll_event one_event = re_event[i];

                    if (one_event.data.fd == m_wake_fd && (one_event.events & READ))
                    {
                        char buf[8];
                        while (1)
                        {
                            if (g_sys_read_fun(m_wake_fd, buf, 8) == 1)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        compactrpc::FdEvent *ptr = (compactrpc::FdEvent *)one_event.data.ptr;
                        if (ptr != nullptr)
                        {
                            int fd = ptr->getFd();

                            if (!((one_event.events & EPOLLIN)) && (!(one_event.events & EPOLLOUT)))
                            {
                                ErrorLog << "socket [" << fd << "] occur other unkown event : [" << one_event.events << "], need unregister this socket";
                                delEvenInLoopThread(fd);
                            }
                            else
                            { // 如果之前注册了协程，pending cor to comm cor_tasks
                                if (ptr->getCoroutine())
                                {
                                    /**
                                     * @brief:第一个协程(epoll返回的时候， 有多个fd就绪， 每个fd 就是一个待执行
                                     *      的协程
                                     * 第一个协程就由当前线程来调度；如果还有其他协程， 再把它们放到全局协程任务队列里
                                     * 这样的目的是：尽量减少访问全局协程队列，毕竟队列要需要加锁的
                                     */
                                    if (!first_cor)
                                    {
                                        first_cor = ptr->getCoroutine();
                                        continue;
                                    }
                                    if (m_reactor_type == SubReactor)
                                    {
                                        delEvenInLoopThread(fd);
                                        ptr->setReactor(nullptr);
                                        CoroutineTaskQueue::GetCoroutineTaskQueue()->push(ptr);
                                    }
                                    else
                                    {
                                        // main reactor, just resume this cor, that is accept_cor and exec Main reactor only have this cor
                                        compactrpc::Coroutine::Resume(ptr->getCoroutine());
                                        if (first_cor)
                                        {
                                            first_cor = nullptr;
                                        }
                                    }
                                }
                                else
                                {
                                    std::function<void()> read_cb;
                                    std::function<void()> write_cb;
                                    read_cb = ptr->getCallback(READ);
                                    write_cb = ptr->getCallback(WRITE);

                                    // if time event arrvie, exec
                                    if (fd == m_timer_fd)
                                    {
                                        read_cb();
                                        continue;
                                    }
                                    if (one_event.events & EPOLLIN)
                                    {
                                        Mutex::Lock lock(m_mutex);
                                        m_pending_task.push_back(read_cb);
                                    }
                                    if (one_event.events & EPOLLOUT)
                                    {
                                        Mutex::Lock lock(m_mutex);
                                        m_pending_task.push_back(write_cb);
                                    }
                                }
                            }
                        }
                    }
                }

                std::map<int, epoll_event> tmp_add;
                std::vector<int> tmp_del;

                {
                    Mutex::Lock lock(m_mutex);
                    tmp_add.swap(m_pending_add_fds);
                    m_pending_add_fds.clear();

                    tmp_del.swap(m_pending_del_fds);
                    m_pending_del_fds.clear();
                }

                for (auto it = tmp_add.begin(); it != tmp_add.end(); it++)
                {
                    addEventInLoopThread(it->first, it->second);
                }
                for (auto it = tmp_del.begin(); it != tmp_del.end(); it++)
                {
                    delEvenInLoopThread(*it);
                }
            }
        }
        DubugLog << "reactor loo end";
        m_is_looping = false;
    }

    void Reactor::stop()
    {
        if (!m_stop_flag && m_is_looping)
        {
            m_stop_flag = true;
            wakeup();
        }
    }

    void Reactor::addTask(std::function<void()> task, bool is_wake)
    {
        {
            Mutex::Lock lock(m_mutex);
            m_pending_task.push_back(task);
        }
        if (is_wake)
        {
            wakeup();
        }
    }

    void Reactor::addTask(std::vector<std::function<void()>> tasks, bool is_wakeup)
    {
        if (tasks.size() == 0)
            return;
        {
            Mutex::Lock lock(m_mutex);
            m_pending_task.insert(m_pending_task.end(), tasks.begin(), tasks.end());
        }
        if (is_wakeup)
        {
            wakeup();
        }
    }

    void Reactor::addCoroutine(compactrpc::Coroutine::ptr cor, bool is_wakeup /*true*/)
    {
        auto func = [cor]()
        {
            compactrpc::Coroutine::Resume(cor.get());
        };
        addTask(func, is_wakeup);
    }

    Timer *Reactor::getTimer()
    {
        if (!m_timer)
        {
            m_timer = new Timer(this);
            m_timer_fd = m_timer->getFd();
        }
        return m_timer;
    }

    pid_t Reactor::getTid()
    {
        return m_tid;
    }

    void Reactor::setReactorType(ReactorType type)
    {
        m_reactor_type = type;
    }

    CoroutineTaskQueue *CoroutineTaskQueue::GetCoroutineTaskQueue()
    {
        if (t_coroutine_task_queue)
        {
            return t_coroutine_task_queue;
        }
        t_coroutine_task_queue = new CoroutineTaskQueue();
        return t_coroutine_task_queue;
    }

    void CoroutineTaskQueue::push(FdEvent *cor)
    {
        Mutex::Lock lock(m_mutex);
        m_task.push(cor);
        lock.unlock();
    }

    FdEvent *CoroutineTaskQueue::pop()
    {
        FdEvent *fd_ptr = nullptr;
        Mutex::Lock lock(m_mutex);
        if (m_task.size() >= 1)
        {
            fd_ptr = m_task.front();
            m_task.pop();
        }
        lock.unlock();
        return fd_ptr;
    }
}