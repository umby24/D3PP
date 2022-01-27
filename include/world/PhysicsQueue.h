//
// Created by Wande on 1/26/2022.
//

#ifndef D3PP_PHYSICSQUEUE_H
#define D3PP_PHYSICSQUEUE_H

#include <queue>
#include <mutex>
#include "world/IUniqueQueue.h"
#include "world/TimeQueueItem.h"

namespace D3PP::world {
    class PhysicsQueue : public IUniqueQueue {
    public:
        explicit PhysicsQueue(const Common::Vector3S& size);
        bool TryDequeue(TimeQueueItem& out);
        void TryQueue(const D3PP::world::TimeQueueItem &in);
        void Clear();
    private:
        std::mutex m_accessLock;
        std::queue<TimeQueueItem> m_PhysicsQueue;
        void TryDequeue_();
    };
}

#endif //D3PP_PHYSICSQUEUE_H
