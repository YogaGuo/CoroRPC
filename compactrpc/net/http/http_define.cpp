/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-13 16:47:39
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-13 16:57:04
 */
#include <string>
#include <sstream>
#include "compactrpc/net/http/http_define.h"

namespace compactrpc
{
    std::string g_CRLF = "\r\n";
    std::string g_CRLF_DOUBLE = "\r\n\r\n";

    std::string content_type_text = "text/html;charset=utf-8";
    const char *default_html_template = "<html><body><h1>%s</h1><p>%s</p></body></html>";

    const char *httpCodeToString(const int code)
    {
        switch (code)
        {
        case HTTP_OK:
            return "OK";

        case HTTP_BADREQUEST:
            return "Bad Request";

        case HTTP_FORBIDDEN:
            return "Forbidden";

        case HTTP_NOTFOUND:
            return "Not Found";

        case HTTP_INTERERROR:
            return "Internal Server Error";

        default:
            return "UnKnown code";
        }
    }

    std::string HttpHeaderComm::getValue(const std::string &key)
    {
        return m_maps[key.c_str()];
    }

    int HttpHeaderComm::getHeaderToTalLength()
    {
        int len = 0;
        for (auto it : m_maps)
        {
            len += it.first.length() + 1 + it.second.length() + 2;
        }
        return len;
    }

    void HttpHeaderComm::setKeyValue(const std::string &key, const std::string &value)
    {
        m_maps[key.c_str()] = value;
    }

    std::string HttpHeaderComm::toHttpString()
    {
        std::stringstream ss;
        for (auto it : m_maps)
        {
            ss << it.first << ":" << it.second << "\r\n";
        }
        return ss.str();
    }
}