/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-09 21:16:54
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-11 12:56:38
 */
#ifndef COMPACTRPC_MEMORY_H
#define COMPACTRPC_MEMORY_H

#include <memory>
#include <vector>
#include <atomic>
#include "compactrpc/comm/Mutex.h"
namespace compactrpc
{
    class Memory
    {
    public:
        typedef std::shared_ptr<Memory> ptr;

        Memory(int block_size, int block_count);
        ~Memory();

        int getRefCount();

        char *getStart();

        char *getEnd();

        char *getBlock();

        bool hasBlock(char *);

        void backBlock(char *);

    private:
        int m_block_size{0};
        int m_block_count{0};

        int m_size{0};
        char *m_start{nullptr};
        char *m_end{nullptr};

        std::atomic<int> m_ref_counts{0};
        std::vector<bool> m_blocks;
        Mutex m_mutex;
    };
}
#endif