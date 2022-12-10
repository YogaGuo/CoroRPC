#ifndef COMPACTRPC_COCTX_H
#define COMPACTRPC_COCTX_H

namespace compactrpc
{

    enum
    {
        KRBP = 6,     // rbp
        KRDI = 7,     // rdi, 第一个函数参数
        KRSI = 8,     // rsi, 第二个函数参数
        KRETAddr = 9, // 函数跳转后即将要执行的指令地址， 即rip的值
        KRSP = 13,    // rsp
    };
    struct coctx
    {
        void *regs[14];
    };
    extern "C"
    {
        //
        extern void coctx_swap(coctx *, coctx *) asm("coctx_swap");
    };
}
#endif