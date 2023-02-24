/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-09 22:08:03
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-16 11:42:13
 */
#include <pthread.h>
#include <memory>
#include "compactrpc/comm/Mutex.h"
#include "compactrpc/net/reactor.h"
#include "compactrpc/comm/log.h"
#include "compactrpc/coroutine/coroutine.h"
#include "compactrpc/coroutine/coroutine_hook.h"

// this file copy form sylar

namespace compactrpc
{

    // void CououtineLock() {
    //   // disable coroutine swap, that's means couroutine can't yield until unlock
    //   Coroutine::SetCoroutineSwapFlag(false);
    // }

    // void CououtineUnLock() {
    //   Coroutine::SetCoroutineSwapFlag(true);
    // }

    CoroutineMutex::CoroutineMutex() {}

    CoroutineMutex::~CoroutineMutex() {}

    void CoroutineMutex::lock()
    {

        if (Coroutine::IsMainCoroutine())
        {
            ErrorLog << "main coroutine can't use coroutine mutex";
            return;
        }
        if (!m_lock)
        {
            m_lock = true;
        }
        else
        {
            // beacuse can't get lock, so should yield current cor

            Coroutine *cor = Coroutine::GetCurrentCoroutine();
            // add to tasks, wait next reactor back to resume this coroutine
            std::shared_ptr<Coroutine> tmp(cor);
            compactrpc::Reactor::GetReactor()->addCoroutine(tmp, false);
            Coroutine::Yield();
        }
    }

    void CoroutineMutex::unlock()
    {
        if (Coroutine::IsMainCoroutine())
        {
            ErrorLog << "main coroutine can't use coroutine mutex";
            return;
        }
        if (m_lock)
        {
            m_lock = false;
        }
    }

}