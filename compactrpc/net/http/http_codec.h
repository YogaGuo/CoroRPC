/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-13 16:35:56
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-19 11:37:11
 */
#ifndef COMPACTRPC_NET_HTTP_HTTP_CODEC_H
#define COMPACTRPC_NET_HTTP_HTTP_CODEC_H

#include <map>
#include <string>
#include "compactrpc/net/abstract_data.h"
#include "compactrpc/net/abstract_codec.h"
#include "compactrpc/net/http/http_request.h"

namespace compactrpc
{
    class HttpCodec : public AbstractCodec
    {
    public:
        HttpCodec();

        ~HttpCodec();

        void encode(TcpBuffer *buf, AbstractData *data) override;

        void decode(TcpBuffer *buf, AbstractData *data) override;

        ProtocalType getProtocalType();

    private:
        bool parseHttpRequestLine(HttpRequest *request, const std::string &tmp);
        bool parseHttpRequestHeader(HttpRequest *request, const std::string &tmp);
        bool parseHttpRequestContent(HttpRequest *request, const std::string &tmp);
    };
}
#endif