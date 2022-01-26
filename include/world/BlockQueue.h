//
// Created by Wande on 1/25/2022.
//

#ifndef D3PP_BLOCKQUEUE_H
#define D3PP_BLOCKQUEUE_H

#include <vector>
#include <mutex>
#include "common/Vectors.h"

namespace D3PP::world {
    class BlockQueue {
    public:
        BlockQueue(const D3PP::Common::Vector3S &size);


    private:
        std::vector<unsigned char> _queueMask;
        std::mutex accessMutex;

        bool IsQueued(const Common::Vector3S& location);
        void Queue(const Common::Vector3S& location);

    };
}
#endif //D3PP_BLOCKQUEUE_H
