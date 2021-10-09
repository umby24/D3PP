#include "common/Configuration.h"
#include "json.hpp"

using json = nlohmann::json;
NetworkSettings Configuration::NetSettings { 32, 25565, true, false};
GeneralSettings Configuration::GenSettings { "D3PP Server", "Welcome to D3PP!", "INFO", 160, 3, true};


Configuration::Configuration() {
    this->Interval = std::chrono::seconds(1);
    this->Main = [this] { MainFunc(); };
    this->Setup = [this] { Load(); };
    this->Teardown = [this] { Save(); };

    TaskScheduler::RegisterTask("Configuration", *this);
}

void Configuration::Load() {

}

void Configuration::Save() {

}

void Configuration::MainFunc() {

}