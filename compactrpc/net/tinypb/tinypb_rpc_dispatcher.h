/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-02-19 15:39:43
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-19 16:43:52
 */
/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-02-19 15:39:43
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-19 16:07:54
 */
#ifndef COMPACTRPC_NET_TINYPB_TINYPB_RPC_DISPATCHER_H
#define COMPACTRPC_NET_TINYPB_TINYPB_RPC_DISPATCHER_H

#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <map>
#include <memory>
#include "compactrpc/net/abstract_dispatch.h"
#include "compactrpc/net/abstract_data.h"

namespace compactrpc
{
    class TinyPbRpcDispacther : public AbstractDispatcher
    {
    public:
        typedef std::shared_ptr<google::protobuf::Service> service_ptr;

        TinyPbRpcDispacther() = default;
        ~TinyPbRpcDispacther() = default;

        void dispatch(AbstractData *data, TcpConnection *conn);

        bool parseServiceFullName(const std::string &full_name, const std::string &service_name, const std::string &method_name);

        void registerService(service_ptr service);

    public:
        /**
         * @brief all service should be register on there before start;
         *
         */
        std::map<std::string, service_ptr> m_service_map;
    };
}
#endif