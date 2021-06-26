#ifndef D3PP_CPE_H
#define D3PP_CPE_H

#include <vector>
#include <map>
#include <string>

#include "Network.h"
#include "Packets.h"

class NetworkClient;

class CPE {
    public:
    static void PreLoginExtensions(std::shared_ptr<NetworkClient> client);
    //static std::map<std::string, int> SupportedExtensions { std::pair<std::string, int>("CustomBlocks", 1)};
};
#endif