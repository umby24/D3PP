#pragma once

#include "common/Vectors.h"

namespace D3PP::Common {
    struct UndoItem {
        Vector3S Location;
        unsigned char OldBlock;
        unsigned char NewBlock;
    };
}