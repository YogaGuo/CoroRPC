/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-11 14:25:55
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-22 15:11:37
 */
#include <assert.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "compactrpc/coroutine/coroutine_hook.h"

/**
 * @brief
 *  name = read
 *  read_fun_ptr_t g_sys_read_fun = (read_fun_ptr_t)dlsym(READ_NEXT, "read");
 */
#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, "#name");

HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(connect);
HOOK_SYS_FUNC(socket);
HOOK_SYS_FUNC(sleep);

namespace compactrpc
{
    extern compactrpc::Config::ptr gRpcConfig;

    static bool g_hook{true};

    void SetHook(bool v)
    {
        g_hook = v;
    }

    void toEpoll(compactrpc::FdEvent::ptr fd_event, int events)
    {
        compactrpc::Coroutine *cur_cor = compactrpc::Coroutine::GetCurrentCoroutine();
        if (events & compactrpc::IOEvent::READ)
        {
            DebugLog << "fd :[" << fd_event->getFd() << "], register read event to epoll";

            fd_event->setCoroutine(cur_cor);
            fd_event->addListenEvent(compactrpc::IOEvent::READ);
        }
        if (events & compactrpc::IOEvent::WRITE)
        {
            DebugLog << "fd :[" << fd_event->getFd() << "], register write event to epoll";

            fd_event->setCoroutine(cur_cor);
            fd_event->addListenEvent(compactrpc::IOEvent::WRITE);
        }
    }

    ssize_t read_hook(int fd, void *buf, size_t count)
    {
        DebugLog << "this is hook read";
        if (compactrpc::Coroutine::IsMainCoroutine())
        {
            DebugLog << "hook disable, call sys read func";
            return g_sys_read_fun(fd, buf, count);
        }

        compactrpc::FdEvent::ptr fd_event = compactrpc::FdEventContianer::GetFdContianer()->getFdEvent(fd);
        if (fd_event->getReactor() == nullptr)
        {
            fd_event->setReactor(compactrpc::Reactor::GetReactor());
        }
        fd_event->setNonBlock();

        /**
         * @brief 先读，如果成功返回大于0的n, 说明读成功了， 不用在后序注册到reactor
         *
         */
        ssize_t n = g_sys_read_fun(fd, buf, count);
        if (n > 0)
            return n;

        toEpoll(fd_event, compactrpc::IOEvent::READ);

        DebugLog << "read func to yield";

        compactrpc::Coroutine::Yield();

        fd_event->delListenEvent(compactrpc::IOEvent::READ);
        fd_event->clearCoroutine();

        DebugLog << "read_hook yield back, ready to exec sys_read";
        return g_sys_read_fun(fd, buf, count);
    }

    ssize_t write_hook(int fd, const void *buf, size_t count)
    {
        DebugLog << "this is hook write";
        if (compactrpc::Coroutine::IsMainCoroutine())
        {
            DebugLog << "hook disable, call sys_write_func";
            return g_sys_write_fun(fd, buf, count);
        }

        compactrpc::FdEvent::ptr fd_event = compactrpc::FdEventContianer::GetFdContianer()->getFdEvent(fd);
        if (fd_event->getReactor() == nullptr)
        {
            fd_event->setReactor(compactrpc::Reactor::GetReactor());
        }
        fd_event->setNonBlock();

        /**
         * @brief 先写，如果成功返回大于0的n, 说明写成功了， 不用在后序注册到reactor
         *
         */
        ssize_t n = g_sys_write_fun(fd, buf, count);
        if (n > 0)
            return n;

        toEpoll(fd_event, compactrpc::IOEvent::READ);

        DebugLog << "write func to yield";

        compactrpc::Coroutine::Yield();

        fd_event->delListenEvent(compactrpc::IOEvent::WRITE);
        fd_event->clearCoroutine();

        DebugLog << "read_hook yield back, ready to exec sys_write";
        return g_sys_write_fun(fd, buf, count);
    }

    int accept_hook(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    {
        DebugLog << "this is accept_hook";
        if (compactrpc::Coroutine::IsMainCoroutine())
        {
            DebugLog << "hook disable, call sys_accept_func";
            return g_sys_accept_fun(sockfd, addr, addrlen);
        }

        compactrpc::FdEvent::ptr fd_event = compactrpc::FdEventContianer::GetFdContianer()->getFdEvent(sockfd);
        if (fd_event->getReactor() == nullptr)
        {
            fd_event->setReactor(compactrpc::Reactor::GetReactor());
        }

        fd_event->setNonBlock();

        ssize_t n = g_sys_accept_fun(sockfd, addr, addrlen);
        if (n > 0)
            return n;

        toEpoll(fd_event, compactrpc::IOEvent::READ);

        DebugLog << "accept_hook to yield";

        compactrpc::Coroutine::Yield();

        fd_event->delListenEvent(compactrpc::IOEvent::READ);
        fd_event->clearCoroutine();

        DebugLog << "accept_hook yield back, ready to exec sys_accept";
        return g_sys_accept_fun(sockfd, addr, addrlen);
    }

    int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        DebugLog << "this is connect_hook";
        if (compactrpc::Coroutine::IsMainCoroutine())
        {
            DebugLog << "hook disable, call sys_connect_func";
            return g_sys_connect_fun(sockfd, addr, addrlen);
        }

        compactrpc::FdEvent::ptr fd_event = compactrpc::FdEventContianer::GetFdContianer()->getFdEvent(sockfd);
        if (fd_event->getReactor() == nullptr)
        {
            fd_event->setReactor(compactrpc::Reactor::GetReactor());
        }

        compactrpc::Coroutine::ptr cur_cor = compactrpc::Coroutine::GetCurrentCoroutine();
        fd_event->setNonBlock();

        ssize_t n = g_sys_connect_fun(sockfd, addr, addrlen);
        if (n == 0)
        {
            DebugLog << "direct connect succ, return";
            return n;
        }

        else if (errno != EINPROGRESS)
        {
            DebugLog << "connect error and error is't EINPROGRESS, error = " << strerror(errno);
            return n;
        }

        DebugLog << "error = EINPROGRESS";

        toEpoll(fd_event, compactrpc::IOEvent::WRITE);

        // 是否超时
        bool is_timeout = false;

        /*
         *  To do: timeEvent setting!
         */
    }

    unsigned int sleep_hook(unsigned int seconds)
    {
    }

    extern "C"
    {

        int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
        {
            if (!compactrpc::g_hook)
            {
                return g_sys_accept_fun(sockfd, addr, addrlen);
            }
            else
            {
                return compactrpc::accept_hook(sockfd, addr, addrlen);
            }
        }

        ssize_t read(int fd, void *buf, size_t count)
        {
            if (!compactrpc::g_hook)
            {
                return g_sys_read_fun(fd, buf, count);
            }
            else
            {
                return compactrpc::read_hook(fd, buf, count);
            }
        }

        ssize_t write(int fd, const void *buf, size_t count)
        {
            if (!compactrpc::g_hook)
            {
                return g_sys_write_fun(fd, buf, count);
            }
            else
            {
                return compactrpc::write_hook(fd, buf, count);
            }
        }

        int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
        {
            if (!compactrpc::g_hook)
            {
                return g_sys_connect_fun(sockfd, addr, addrlen);
            }
            else
            {
                return compactrpc::connect_hook(sockfd, addr, addrlen);
            }
        }

        unsigned int sleep(unsigned int seconds)
        {
            if (compactrpc::g_hook)
            {
                return g_sys_sleep_fun(seconds);
            }
            else
            {
                return compactrpc::sleep_hook(seconds);
            }
        }
    }
}