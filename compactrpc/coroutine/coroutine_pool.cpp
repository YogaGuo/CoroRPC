/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-10 14:30:13
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-11 14:22:44
 */

#include <sys/mman.h>
#include "compactrpc/comm/config.h"
#include "compactrpc/comm/log.h"
#include "compactrpc/coroutine/coroutine_pool.h"

namespace compactrpc
{
    extern compactrpc::Config::ptr gRpcConfig;

    static CoroutinePool *t_coroutine_container_ptr = nullptr;

    CoroutinePool *getCoroutinePool()
    {
        if (!t_coroutine_container_ptr)
            t_coroutine_container_ptr = new CoroutinePool(gRpcConfig->m_cor_size,
                                                          gRpcConfig->m_cor_stack_size);
        return t_coroutine_container_ptr;
    }
    // 1024 * 128 b--->128K
    CoroutinePool::CoroutinePool(int pool_size, int stack_size)
        : m_pool_size(pool_size), m_stack_size(stack_size)
    {
        // set main cor first
        Coroutine::GetCurrentCoroutine();

        m_memory_pool.push_back(std::make_shared<Memory>(stack_size, pool_size));

        Memory::ptr tmp = m_memory_pool[0];
        for (int i = 0; i < pool_size; i++)
        {
            Coroutine::ptr cor = std::make_shared<Coroutine>(stack_size, tmp->getBlock());
            cor->setIndex(i);
            m_free_cors.push_back({cor, false});
        }
    }
    CoroutinePool::~CoroutinePool()
    {
    }

    Coroutine::ptr CoroutinePool::getCoroutineInstance()
    {
        Mutex::Lock lock(m_mutex);
        for (int i = 0; i < m_pool_size; i++)
        {
            if (!m_free_cors[i].first->getIsInCorFunc() && !m_free_cors[i].second)
            {
                m_free_cors[i].second = true;
                Coroutine::ptr cor = m_free_cors[i].first;
                lock.unlock();
                return cor;
            }
        }

        for (size_t i = 1; i < m_memory_pool.size(); i++)
        {
            char *tmp = m_memory_pool[i]->getBlock();
            if (tmp)
            {
                Coroutine::ptr cor = std::make_shared<Coroutine>(m_stack_size, tmp);
                return cor;
            }
            m_memory_pool.push_back(std::make_shared<Memory>(m_stack_size, m_pool_size));
            return std::make_shared<Coroutine>(m_stack_size,
                                               m_memory_pool[m_memory_pool.size() - 1]->getBlock());
        }
    }

    void CoroutinePool::returnCoroutine(Coroutine::ptr cor)
    {
        int i = cor->getIndex();
        if (i >= 0 && i < m_pool_size)
        {
            m_free_cors[i].second = false;
        }
        else
        {
            for (int i = 1; i < m_memory_pool.size(); i++)
            {
                if (m_memory_pool[i]->hasBlock(cor->getStackPtr()))
                {
                    m_memory_pool[i]->backBlock(cor->getStackPtr());
                }
            }
        }
    }
}
