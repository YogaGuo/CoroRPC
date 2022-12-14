#ifndef COMACTRPC_ABSTRACT_SLOT_H
#define COMACTRPC_ABSTRACT_SLOT_H

#include <memory>
#include <functional>

namespace compactrpc
{

    template <class T>
    class AbstractSlot
    {
    public:
        typedef std::shared_ptr<AbstractSlot> ptr;
        typedef std::shared_ptr<T> sharedPtr;
        typedef std::weak_ptr<T> weakPtr;

        AbstractSlot(weakPtr weak_ptr, std::function<void(sharedPtr)> cb)
            : m_weak_ptr(weak_ptr), m_cb(cb) {}

        ~AbstractSlot()
        {
            sharedPtr ptr = m_weak_ptr.lock();
            if (ptr)
            {
                m_cb(ptr);
            }
        }

    private:
        weakPtr m_weak_ptr;
        std::function<void(sharedPtr)> m_cb;
    };
}
#endif