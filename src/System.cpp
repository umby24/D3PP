//
// Created by Wande on 2/25/2021.
//

#include "System.h"

const std::string MODULE_NAME = "System";
System* System::Instance_ = nullptr;
bool System::IsRunning = false;
time_t System::startTime = time(nullptr);
std::mutex System::mainMutex;

System* System::GetInstance() {
    if (Instance_ == nullptr)
        Instance_ = new System();
    
    return Instance_;
}