#ifndef D3PP_CPE_H
#define D3PP_CPE_H

#include <memory>

class NetworkClient;

class CPE {
    public:
    static void PreLoginExtensions(std::shared_ptr<NetworkClient> client);
    //static std::map<std::string, int> SupportedExtensions { std::pair<std::string, int>("CustomBlocks", 1)};
};
#endif