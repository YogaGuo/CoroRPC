/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-13 16:41:12
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-14 21:46:21
 */
#ifndef COPMACTPRC_NET_HTTP_HTTP_DEFINE_H
#define COPMACTPRC_NET_HTTP_HTTP_DEFINE_H

#include <string>
#include <map>

namespace compactrpc
{
    extern std::string g_CRLF;
    extern std::string g_CRLF_DOUBLE;
    extern std::string content_type_text;
    extern const char *default_html_template;

    enum HttpMethod
    {
        GET = 1,
        POST = 2,
    };

    enum HttpCode
    {
        HTTP_OK = 200,
        HTTP_BADREQUEST = 400,
        HTTP_FORBIDDEN = 403,
        HTTP_NOTFOUND = 404,
        HTTP_INTERERROR = 500,
    };

    const char *httpCodeToString(const int code);

    class HttpHeaderComm
    {
    public:
        HttpHeaderComm() = default;
        virtual ~HttpHeaderComm() = default;

        int getHeaderToTalLength();

        std::string getValue(const std::string &key);

        void setKeyValue(const std::string &key, const std::string &value);

        std::string toHttpString();

    public:
        std::map<std::string, std::string> m_maps;
    };

    class HttpRequestHeader : public HttpHeaderComm
    {
    };
    class HttpResponseHeader : public HttpHeaderComm
    {
    };
}
#endif