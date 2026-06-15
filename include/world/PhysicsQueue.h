//
// Created by Wande on 1/26/2022.
//

#ifndef D3PP_PHYSICSQUEUE_H
#define D3PP_PHYSICSQUEUE_H

#include <queue>
#include <vector>
#include <functional>
#include <mutex>
#include "world/IUniqueQueue.h"
#include "world/TimeQueueItem.h"

namespace D3PP::world {
    class PhysicsQueue : public IUniqueQueue {
    public:
        explicit PhysicsQueue(const Common::Vector3S& size);
        int GetSize() const override { return m_PhysicsQueue.size(); }
        bool TryDequeue(TimeQueueItem& out);
        void TryQueue(const TimeQueueItem &in);
        void Clear();
    private:
        std::mutex m_accessLock;
        // -- Min-heap by Time: the soonest-due item is always on top.
        std::priority_queue<TimeQueueItem, std::vector<TimeQueueItem>, std::greater<TimeQueueItem>> m_PhysicsQueue;
        void TryDequeue_();
    };
}

#endif //D3PP_PHYSICSQUEUE_H
