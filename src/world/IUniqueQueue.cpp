//
// Created by Wande on 1/25/2022.
//
#include "world/IUniqueQueue.h"
#include <cmath>

namespace D3PP::world {
    IUniqueQueue::IUniqueQueue(Common::Vector3S size) : m_queueData((size.X*size.Y*size.Z)/8), m_size{} {
        m_size = size;
    }

    bool IUniqueQueue::IsQueued(const Common::Vector3S& loc) {
        int offset = GetOffset(loc);
        int offsetTop = std::ceil(offset / 8);

        if (offsetTop > m_queueData.size())
            return false;

        return (m_queueData.at(offsetTop) & (1 << (offset % 8))) > 0;
    }

    void IUniqueQueue::Queue(const Common::Vector3S& loc) {
        int offset = GetOffset(loc);
        int offsetTop = std::ceil(offset / 8);
        if (offsetTop > m_queueData.size())
            return;
        m_queueData.at(offsetTop) |= (1 << (offset % 8));
    }

    int IUniqueQueue::GetOffset(const Common::Vector3S& loc) const {
       return (loc.X + loc.Y * m_size.X + loc.Z * m_size.X * m_size.Y);
    }

    void IUniqueQueue::Dequeue(const Common::Vector3S& loc) {
        int offset = GetOffset(loc);
        int offsetTop = std::ceil(offset / 8);

        if (offsetTop > m_queueData.size())
            return;

        m_queueData.at(offsetTop) &= ~(1 << (offset % 8));
    }
}
