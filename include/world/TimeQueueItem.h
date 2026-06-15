//
// Created by Wande on 1/26/2022.
//

#ifndef D3PP_TIMEQUEUEITEM_H
#define D3PP_TIMEQUEUEITEM_H
#include <chrono>
#include "common/Vectors.h"

namespace D3PP::world {
    struct TimeQueueItem {
        Common::Vector3S Location;
        std::chrono::time_point<std::chrono::steady_clock> Time;

        bool operator()(const TimeQueueItem &a, const TimeQueueItem &b) const {
            return a.Time < b.Time;
        }

        bool operator< (const TimeQueueItem &b) const {
            return Time < b.Time;
        }

        bool operator> (const TimeQueueItem &b) const {
            return Time > b.Time;
        }
    };
}

#endif //D3PP_TIMEQUEUEITEM_H
