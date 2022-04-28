//
// Created by Wande on 1/25/2022.
//
#include "world/BlockChangeQueue.h"


D3PP::world::BlockChangeQueue::BlockChangeQueue(const D3PP::Common::Vector3S &size) : IUniqueQueue(size) {}

bool D3PP::world::BlockChangeQueue::TryDequeue(D3PP::world::ChangeQueueItem &out) {
    std::scoped_lock<std::mutex> pLock(m_accessLock);
    if (m_ChangeQueue.empty())
        return false;

    out = ChangeQueueItem(m_ChangeQueue.top()); // -- Const Ref, need to use copy constructor.
    m_ChangeQueue.pop();
    Dequeue(out.Location);
    return true;
}

void D3PP::world::BlockChangeQueue::TryQueue(const D3PP::world::ChangeQueueItem &in) {
    if (IsQueued(in.Location))
        return;

    std::scoped_lock<std::mutex> pLock(m_accessLock);
    m_ChangeQueue.push(in);
    Queue(in.Location);
}

void D3PP::world::BlockChangeQueue::Clear() {
    std::scoped_lock<std::mutex> pLock(m_accessLock);
    while (!m_ChangeQueue.empty()) {
        TryDequeue_();
    }
}

void D3PP::world::BlockChangeQueue::TryDequeue_() {
    if (m_ChangeQueue.empty())
        return;

    auto meh = ChangeQueueItem(m_ChangeQueue.top()); // -- Const Ref, need to use copy constructor.
    m_ChangeQueue.pop();
    Dequeue(meh.Location);
}


