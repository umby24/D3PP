//
// Created by Wande on 1/25/2022.
//

#ifndef D3PP_IUNIQUEQUEUE_H
#define D3PP_IUNIQUEQUEUE_H

#include <vector>
#include "common/Vectors.h"
// -- Creating an interface type so we can have two separate kinds of queue
// -- "PhysicsUniqueQueue" and "ChangeUniqueQueue".
// -- They should both use the same backing code to determine if a given coordinate is queued already
// -- But they will have a different backing type for what their actual queue items are :)
namespace D3PP::world {
    class IUniqueQueue {
    public:
        IUniqueQueue(Common::Vector3S size);
        virtual ~IUniqueQueue() = default;

        [[nodiscard]] int GetOffset(const Common::Vector3S& loc) const;

        bool IsQueued(const Common::Vector3S& loc);

        void Queue(const Common::Vector3S& loc);
        void Dequeue(const Common::Vector3S& loc);
    private:
        Common::Vector3S m_size{};
        std::vector<unsigned char> m_queueData;
    };

}

#endif //D3PP_IUNIQUEQUEUE_H
