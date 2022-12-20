/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2022-12-20 13:57:46
 * @LastEditors: Yogaguo
 * @LastEditTime: 2022-12-20 15:55:10
 */
#ifndef COMPACTRPC_NET_ADDRESS_H
#define COMPACTRPC_NET_ADDRESS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <string>
#include <netinet/in.h>
#include <memory>

namespace compactrpc
{
    class NetAddress
    {
    public:
        typedef std::shared_ptr<NetAddress> ptr;

        virtual sockaddr *getSockAddr() = 0;

        virtual int getFamily() const = 0;

        virtual std::string toString() const = 0;

        virtual socklen_t getSockLen() const = 0;
    };

    class IPAddress : public NetAddress
    {
    public:
        IPAddress(const std::string &ip, uint16_t port);

        IPAddress(const std::string &addr);

        IPAddress(uint16_t port);

        IPAddress(sockaddr_in addr);

        sockaddr *getSockAddr() override;

        int getFamily() const override;

        std::string toString() const override;

        socklen_t getSockLen() const override;

        std::string getIP() const
        {
            return m_ip;
        }

        int getPort() const
        {
            return m_port;
        }

        static bool CheckValidIPAddr(const std::string &addr);

    private:
        std::string m_ip;

        uint16_t m_port;

        sockaddr_in m_addr;
    };

    class UnixDomainAddress : public NetAddress
    {
    public:
        UnixDomainAddress(std::string &path);

        UnixDomainAddress(sockaddr_un addr);

        sockaddr *getSockAddr() override;

        int getFamily() const override;

        socklen_t getSockLen() const override;

        std::string toString() const override;

        std::string getPath() const
        {
            return m_path;
        }

    private:
        std::string m_path;
        sockaddr_un m_addr;
    };
}
#endif
