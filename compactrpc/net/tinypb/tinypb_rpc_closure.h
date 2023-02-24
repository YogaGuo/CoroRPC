/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-02-19 16:03:42
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-21 11:06:29
 */
#ifndef COPMACTRPC_NET_TINYPB_TINYPB_RPC_CLOSURE_H
#define COPMACTRPC_NET_TINYPB_TINYPB_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
namespace compactrpc
{
    class TinyPbRpcClosure : public google::protobuf::Closure
    {
    public:
        typedef std::shared_ptr<TinyPbRpcClosure> ptr;
        explicit TinyPbRpcClosure(std::function<void()> cb) : m_cb(cb) {}

        ~TinyPbRpcClosure() = default;
        void Run()
        {
            if (m_cb)
            {
                m_cb();
            }
        }

    private:
        std::function<void()> m_cb{nullptr};
    };
}
#endif