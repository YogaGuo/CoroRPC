/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-11 14:25:55
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-14 11:31:06
 */
#ifndef COMPACTRPC_COROUTINE_HOOK_H
#define COMPACTRPC_COROUTINE_HOOK_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "compactrpc/coroutine/coroutine.h"
#include "compactrpc/net/fd_event.h"
#include "compactrpc/net/reactor.h"
#include "compactrpc/net/timer.h"
#include "compactrpc/comm/log.h"
#include "compactrpc/comm/config.h"
typedef ssize_t (*read_fun_ptr_t)(int fd, void *buf, size_t count);

typedef ssize_t (*write_fun_ptr_t)(int fd, const void *buf, size_t count);

typedef int (*accept_fun_ptr_t)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

typedef int (*connect_fun_ptr_t)(int soskfd, const struct sockaddr *addr, socklen_t addrlen);

typedef int (*socket_fun_ptr_t)(int domain, int type, int protocol);

typedef unsigned int (*sleep_fun_ptr_t)(unsigned int seconds);

namespace compactrpc
{
    ssize_t read_hook(int fd, void *buf, size_t count);

    ssize_t write_hook(int fd, const void *buf, size_t count);

    int accept_hook(int sockfa, struct sockaddr *addr, socklen_t *addrlen);

    int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    int socket_hook(int domain, int type, int protocol);

    unsigned int sleep_hook(unsigned int seconds);

    void SetHook(bool v);
}

extern "C"
{
    ssize_t read(int fd, void *buf, size_t count);

    ssize_t write(int fd, const void *buf, size_t count);

    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    unsigned int sleep(unsigned int seconds);
}
#endif