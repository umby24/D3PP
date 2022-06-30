//
// Created by unknown on 2/18/2021.
//

#ifndef D3PP_NETWORK_H
#define D3PP_NETWORK_H

#define NETWORK_CLIENT_TIMEOUT 30


#include <memory>

#include "common/TaskScheduler.h"


class IMinecraftClient;

class Network {
public:
    Network();
    static Network* GetInstance();
    static Network* singleton_;
    static std::shared_ptr<IMinecraftClient> GetClient(int id);
};

#endif //D3PP_NETWORK_H
