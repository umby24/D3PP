//
// Created by Wande on 1/25/2022.
//

#ifndef D3PP_BLOCKCHANGEQUEUE_H
#define D3PP_BLOCKCHANGEQUEUE_H

#include <vector>
#include <mutex>
#include <queue>
#include "IUniqueQueue.h"
#include "world/ChangeQueueItem.h"

namespace D3PP::world {
    class BlockChangeQueue : IUniqueQueue {
    public:
        explicit BlockChangeQueue(const Common::Vector3S& size);
        bool TryDequeue(ChangeQueueItem& out);
        void TryQueue(const ChangeQueueItem &in);
        void Clear();
    private:
        std::mutex m_accessLock;
        std::priority_queue<ChangeQueueItem, std::vector<ChangeQueueItem>, ChangeQueueItem> m_ChangeQueue;
        void TryDequeue_();
    };
}
#endif //D3PP_BLOCKCHANGEQUEUE_H
