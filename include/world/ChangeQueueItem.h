//
// Created by Wande on 1/26/2022.
//

#ifndef D3PP_CHANGEQUEUEITEM_H
#define D3PP_CHANGEQUEUEITEM_H
#include "common/Vectors.h"

namespace D3PP::world {
    struct ChangeQueueItem {
        Common::Vector3S Location;
        unsigned char Priority;
        short OldMaterial;

        bool operator()(const ChangeQueueItem &a, const ChangeQueueItem &b) {
            return a.Priority < b.Priority;
        }

        bool operator< (const ChangeQueueItem &b) const {
            return Priority < b.Priority;
        }

        bool operator> (const ChangeQueueItem &b) const {
            return Priority > b.Priority;
        }
    };
}
#endif //D3PP_CHANGEQUEUEITEM_H
