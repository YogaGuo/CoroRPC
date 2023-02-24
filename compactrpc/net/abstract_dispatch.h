/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-12 21:11:54
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-19 15:48:31
 */
#ifndef COMPACTRPC_NET_ABSTRACT_DISPATCHER_H
#define COMPACTRPC_NET_ABSTRACT_DISPATCHER_H

#include <memory>
#include <google/protobuf/service.h>
#include "compactrpc/net/abstract_data.h"
#include "compactrpc/net/tcp/tcp_connection.h"

namespace compactrpc
{
    class TcpConnection;

    class AbstractDispatcher
    {
    public:
        typedef std::shared_ptr<AbstractDispatcher> ptr;

        AbstractDispatcher(){};

        virtual ~AbstractDispatcher();

        virtual void dispatch(AbstractData *data, TcpConnection *conn) = 0;
    };
}
#endif