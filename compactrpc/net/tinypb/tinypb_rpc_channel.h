/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-02-21 11:07:03
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-21 11:41:24
 */
#ifndef COMPACTRPC_NET_TINYPB_RPC_CHANNEL_H
#define COMPACTRPC_NET_TINYPB_RPC_CHANNEL_H

#include <memory>
#include <google/protobuf/service.h>
#include "compactrpc/net/net_address.h"
#include "compactrpc/net/tcp/tcp_client.h"

namespace compactrpc
{
    class TinyPbRpcChannel : public google::protobuf::RpcChannel
    {
    public:
        typedef std::shared_ptr<TinyPbRpcChannel> ptr;
        TinyPbRpcChannel(NetAddress::ptr addr);
        ~TinyPbRpcChannel() = default;

        void CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller,
                        const google::protobuf::Message *request,
                        google::protobuf::Message *response,
                        google::protobuf::Closure *done) override;

    private:
        NetAddress::ptr m_addr;
    };
}
#endif