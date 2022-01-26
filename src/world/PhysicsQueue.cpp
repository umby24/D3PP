//
// Created by Wande on 1/26/2022.
//

#include <world/PhysicsQueue.h>
#include <mutex>

bool D3PP::world::PhysicsQueue::TryDequeue(TimeQueueItem &out) {
    std::scoped_lock<std::mutex> pLock(m_accessLock);
    if (m_PhysicsQueue.empty())
        return false;

    out = m_PhysicsQueue.front();
    m_PhysicsQueue.pop();
    Dequeue(out.Location);

    return true;
}

void D3PP::world::PhysicsQueue::TryQueue(const D3PP::world::TimeQueueItem &in) {
    if (IsQueued(in.Location))
        return;

    std::scoped_lock<std::mutex> pLock(m_accessLock);
    m_PhysicsQueue.push(in);
    Queue(in.Location);
}

D3PP::world::PhysicsQueue::PhysicsQueue(const D3PP::Common::Vector3S &size) : IUniqueQueue(size) {}

void D3PP::world::PhysicsQueue::Clear() {
    std::scoped_lock<std::mutex> pLock(m_accessLock);
    TimeQueueItem garbage{};

    while (!m_PhysicsQueue.empty()) {
        TryDequeue(garbage);
    }
}
