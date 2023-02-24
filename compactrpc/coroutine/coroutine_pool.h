/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-10 14:30:13
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-22 17:36:27
 */
#include <vector>
#include "compactrpc/coroutine/coroutine.h"
#include "compactrpc/comm/Mutex.h"
#include "compactrpc/coroutine/memory.h"

namespace compactrpc
{
    class CoroutinePool
    {
    public:
        CoroutinePool(int pool_size, int stack_size = 1024 * 128); // 128K

        ~CoroutinePool();

        Coroutine::ptr getCoroutineInstance();

        void returnCoroutine(Coroutine::ptr);

    private:
        int m_pool_size{0};
        int m_stack_size{0};

        // first: cor's ptr.
        // second: false -> can be dispatched, true, can't be dispatched
        std::vector<std::pair<Coroutine::ptr, bool>> m_free_cors;

        Mutex m_mutex;

        std::vector<Memory::ptr> m_memory_pool;
    };

    CoroutinePool *getCoroutinePool()
    {
    }
}