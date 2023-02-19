/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-13 16:38:19
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-14 20:50:27
 */
#ifndef COMPACTRPC_NET_HTTP_HTTP_REQUEST_H
#define COMPACTRPC_NET_HTTP_HTTP_REQUEST_H

#include <string>
#include <memory>
#include <map>
#include "compactrpc/net/abstract_data.h"
#include "compactrpc/net/http/http_define.h"

namespace compactrpc
{
    class HttpRequest : public AbstractData
    {
    public:
        typedef std::shared_ptr<HttpRequest> ptr;

    public:
        HttpMethod m_request_method;
        std::string m_request_path;
        std::string m_request_query;
        std::string m_request_version;
        HttpRequestHeader m_request_header;
        std::string m_request_body;

        std::map<std::string, std::string> m_query_maps;
    };
}
#endif