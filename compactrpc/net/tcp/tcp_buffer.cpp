/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-03 19:41:20
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-14 22:08:42
 */
#include <unistd.h>
#include "compactrpc/net/tcp/tcp_buffer.h"
#include "compactrpc/comm/log.h"

namespace compactrpc
{
    TcpBuffer::TcpBuffer(int size)
    {
        m_buffer.resize(size);
    }

    TcpBuffer::~TcpBuffer() {}

    int TcpBuffer::readAble()
    {
        return m_write_index - m_read_index;
    }

    int TcpBuffer::writeAble()
    {
        return m_buffer.size() - m_write_index;
    }

    int TcpBuffer::readIndex() const
    {
        return m_read_index;
    }

    int TcpBuffer::writeIndex() const
    {
        return m_write_index;
    }

    void TcpBuffer::resizeBuffer(int size)
    {
        std::vector<char> tmp(size);
        int c = std::min(size, readAble());
        memcpy(&tmp[0], &m_buffer[m_read_index], c);

        m_buffer.swap(tmp);
        m_read_index = 0;
        m_write_index = m_read_index + c;
    }

    void TcpBuffer::adjustBuffer()
    {
        if (m_read_index > static_cast<int>(m_buffer.size() / 3))
        {
            std::vector<char> new_buffer(m_buffer.size());

            int count = readAble();
            memcpy(&new_buffer[0], &m_buffer[m_read_index], count);

            m_buffer.swap(new_buffer);
            m_write_index = count;
            m_read_index = 0;
            new_buffer.clear();
        }
    }

    void TcpBuffer::writeToBuffer(const char *buf, int size)
    {
        if (size > writeAble())
        {
            int new_size = (int)(1.5 * (m_write_index + size));
            resizeBuffer(new_size);
        }

        memcpy(&m_buffer[m_write_index], buf, size);
        m_write_index += size;
    }

    void TcpBuffer::readFromBuffer(std::vector<char> &re, int size)
    {
        if (readAble() == 0)
        {
            DubugLog << "read buffer empty!";
            return;
        }

        int read_size = readAble() > size ? size : readAble();
        std::vector<char> tmp(read_size);

        memcpy(&tmp[0], &m_buffer[m_read_index], read_size);
        re.swap(tmp);
        m_read_index += read_size;
        adjustBuffer();
    }

    int TcpBuffer::getSize()
    {
        return m_buffer.size();
    }

    void TcpBuffer::clearBuffer()
    {
        m_buffer.clear();
        m_read_index = 0;
        m_write_index = 0;
    }

    void TcpBuffer::recycleRead(int index)
    {
        int j = m_read_index + index;
        if (j > static_cast<int>(m_buffer.size()))
        {
            ErrorLog << "recycleRead error";
            return;
        }

        m_read_index = j;
        adjustBuffer();
    }

    void TcpBuffer::recycleWrite(int index)
    {
        int j = m_write_index + index;
        if (j > static_cast<int>(m_buffer.size()))
        {
            ErrorLog << "recycleWrite error";
            return;
        }

        m_write_index = j;
        adjustBuffer();
    }

    std::string TcpBuffer::getBufferString()
    {
        int size = readAble();
        std::string re;
        re.resize(size);
        memcpy(&re[0], &m_buffer[m_read_index], readAble());
        return re;
    }

    std::vector<char> TcpBuffer::getBufferVector()
    {
        return m_buffer;
    }
}