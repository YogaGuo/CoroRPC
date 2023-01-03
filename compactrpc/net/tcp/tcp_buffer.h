/*
 * @Description:
 * @Version: 2.0
 * @Autor: Yogaguo
 * @Date: 2023-01-03 19:41:20
 * @LastEditors: Yogaguo
 * @LastEditTime: 2023-01-03 21:35:58
 */
#ifndef COMPACTRPC_TCP_BUFFER
#define COMPACTRPC_TCP_BUFFER
#include <vector>
#include <memory>
#include <string.h>
namespace compactrpc
{
    class TcpBuffer
    {
    public:
        typedef std::shared_ptr<TcpBuffer> ptr;

        explicit TcpBuffer(int size);

        ~TcpBuffer();

        int readAble();

        int writeAble();

        int readIndex() const;

        int writeIndex() const;

        void readFromBuffer(std::vector<char> &re, int size);

        void writeToBuffer(const char &buf, int size);

        void resizeBuffer(int size);

        void clearBuffer();

        int getSize();

        std::vector<char> getBufferVector();

        std::string getBufferString();

        void recycleWrite(int index);

        void recycleRead(int index);

        void adjustBuffer();

    public:
        std::vector<char> m_buffer;

    private:
        int m_read_index{0};

        int m_write_index{0};

        int m_size{0};
    };
};

#endif