/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-14 20:55:38
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-02-19 11:11:40
 */
#ifndef COMPACTRPC_NET_HTTP_HTTP_RESPONSE_H
#define COMPACTRPC_NET_HTTP_HTTP_RESPONSE_H

#include <string>
#include <map>
#include "compactrpc/net/abstract_data.h"
#include "compactrpc/net/http/http_define.h"
namespace compactrpc
{
    class HttpResponse : public AbstractData
    {
    public:
        typedef std::shared_ptr<HttpResponse> ptr;

    public:
        std::string m_response_version;
        int m_response_code;
        std::string m_response_info;
        HttpResponseHeader m_response_header;
        std::string m_response_body;
    };
}
#endif