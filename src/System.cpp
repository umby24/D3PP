//
// Created by Wande on 2/25/2021.
//

#include "System.h"

const std::string MODULE_NAME = "System";
bool System::IsRunning = false;
time_t System::startTime = time(nullptr);
