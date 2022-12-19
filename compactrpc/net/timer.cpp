/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-12 10:15:59
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-15 13:04:48
 */
#include <sys/timerfd.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include "timer.h"
#include "compactrpc/coroutine/coroutine_hook.h"

extern read_fun_ptr_t g_sys_read_fun;

namespace compactrpc
{
    int64_t getNowMs()
    {
        timeval val;
        gettimeofday(&val, nullptr);
        int64_t re = val.tv_sec * 1000 + val.tv_usec / 1000;
        return re;
    }

    Timer::Timer(Reactor *reactor) : FdEvent(reactor)
    {
        }
}