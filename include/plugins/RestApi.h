//
// Created by Wande on 4/19/2022.
//

#ifndef D3PP_RESTAPI_H
#define D3PP_RESTAPI_H
#include <thread>
#include <memory>
#include "common/TaskScheduler.h"

namespace httplib {
    class Server;
}

class RestApi : public TaskItem {
public:
    RestApi();

    void Init();
    void Shutdown();
private:
    std::shared_ptr<httplib::Server> m_restServer;
    std::thread m_serverThread;
    void RunHttpServer() const;
};

#endif //D3PP_RESTAPI_H
