//
// Created by Wande on 2/24/2021.
//

#include "Mem.h"
Mem* Mem::singleton_ = nullptr;
std::mutex Mem::_lock;
const std::string MODULE_NAME = "Mem";

Mem::Mem() {
    this->Interval = std::chrono::milliseconds(10000);
    this->Main = [this] { MainFunc(); };

    TaskScheduler::RegisterTask("Mem", *this);
}

void Mem::MainFunc() {
    struct MemUsageChronic newChronic { MemoryUsage, GetWorkingSetSize(), GetPageFileUsage()};
    _chronics.push_back(newChronic);

    while (_chronics.size() > 100) {
        _chronics.erase(_chronics.begin());
    }

    HtmlStats();
}

int Mem::GetWorkingSetSize() {
    return 0;
}

int Mem::GetPageFileUsage() {
    return 0;
}

char* Mem::Allocate(long size, std::string File, int line, std::string Message) {
    char* mem = new char[size];
    _lock.lock();

    Mem* instance = GetInstance();

    struct MemElement newElement { mem, size, File, line, Message};
    instance->_elements.push_back(newElement);
    instance->MemoryUsage += size;
    _lock.unlock();

    return mem;
}

void Mem::Free(char *memory) {
    bool found;

    _lock.lock();
    Mem* m = GetInstance();
    int i = 0;
    for(auto item : m->_elements) {
        if (item.Memory == memory) {
            delete memory;
            found = true;
            m->MemoryUsage -= item.Size;
            m->_elements.erase(m->_elements.begin() + i);
            break;
        }
        i++;
    }
    if (!found) {
        Logger::LogAdd(MODULE_NAME, "Could not find memory", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    }
    _lock.unlock();
}

Mem *Mem::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new Mem();

    return singleton_;
}

void Mem::HtmlStats() {
    time_t startTime = time(nullptr);
    std::string result = MEM_HTML_TEMPLATE;
    _lock.lock();
    Utils::replaceAll(result, "[MEM]", stringulate(MemoryUsage / 1000000.0));
    Utils::replaceAll(result, "[RAM]", stringulate(GetWorkingSetSize() / 1000000.0));
    Utils::replaceAll(result, "[PGE]", stringulate(GetPageFileUsage() / 1000000.0));
    Utils::replaceAll(result, "[UNK]", stringulate((GetPageFileUsage() - MemoryUsage) / 1000000.0));
    Utils::replaceAll(result, "[ALLOCS]", stringulate(_elements.size()));
    // -- Chronic Graph Generation
    std::string chronicTable;
    int max = 1;
    for(auto item : _chronics) {
        if (max < item.Mem)
            max= item.Mem;
        if (max < item.Ram)
            max = item.Ram;
        if (max< item.Page)
            max= item.Page;
    }
    for(auto item : _chronics) {
        float factor = item.Mem / max; // -- mem
        chronicTable += R"(<td bgcolor="#000000" valign="bottom" height="500">)";
        chronicTable += "<table border=0 cellspacing=0><td bgcolor=\"#FF0000\" height=\"" + stringulate(factor * 500) + "\" width=\"3\"> </td></table>";
        chronicTable += "</td>";
        factor = item.Ram / max;
        chronicTable += R"(<td bgcolor="#000000" valign="bottom" height="500">)";
        chronicTable += "<table border=0 cellspacing=0><td bgcolor=\"#FFFF00\" height=\"" + stringulate(factor * 500) + "\" width=\"3\"> </td></table>";
        chronicTable += "</td>";
        factor = item.Page / max;
        chronicTable += R"(<td bgcolor="#000000" valign="bottom" height="500">)";
        chronicTable += "<table border=0 cellspacing=0><td bgcolor=\"#FFFFFF\" height=\"" + stringulate(factor * 500) + "\" width=\"3\"> </td></table>";
        chronicTable += "</td>";
    }
    Utils::replaceAll(result, "[CHRONIC_TABLE]", chronicTable);
    // -- Fragmentation Graph Generation
    std::string fragGraph;
    double Pos = 0;
    double maxPos = pow(2, 32);
    double free = 0;
    for(auto item : _elements) {
        auto memAddr = reinterpret_cast<uintptr_t>(item.Memory);
        free = round((memAddr - Pos) / (maxPos*800));
        double size = round(item.Size / (maxPos*800));
        Pos = memAddr + size;
        fragGraph += "<td bgcolor=\"#000000\" width=\"" + stringulate(free)+ "\" height=\"20\"> </td>";
        fragGraph += "<td bgcolor=\"#FF0000\" width=\"" + stringulate(size)+ "\" height=\"20\"> </td>";
    }
    free= round((maxPos - Pos) / (maxPos*800));
    fragGraph += "<td bgcolor=\"#000000\" width=\"" + stringulate(free)+ "\" height=\"20\"> </td>";
    Utils::replaceAll(result, "[FRAG_TABLE]", chronicTable);
    // -- Elements list
    std::string elementsList;

    for (auto item : _elements) {
        elementsList += "<tr>\n";
        elementsList += "<td>" + stringulate(reinterpret_cast<uintptr_t>(item.Memory)) + "</td>\n";
        elementsList += "<td>" + stringulate((item.Size / 1000000.0)) + "MB</td>\n";
        elementsList += "<td>" + item.File + "</td>\n";
        elementsList += "<td>" + stringulate(item.Line) + "</td>\n";
        elementsList += "<td>" + item.Message + "</td>\n";
        elementsList += "</tr>\n";
    }
    Utils::replaceAll(result, "[ELEMENT_TABLE]", elementsList);
    time_t finishTime = time(nullptr);
    long duration = finishTime - startTime;
    char buffer[255];
    strftime(buffer, sizeof(buffer), "%H:%M:%S  %m-%d-%Y", localtime(reinterpret_cast<const time_t *>(&finishTime)));
    std::string meh(buffer);
    Utils::replaceAll(result, "[GEN_TIME]", stringulate(duration));
    Utils::replaceAll(result, "[GEN_TIMESTAMP]", meh);

    Files* files = Files::GetInstance();
    std::string memFile = files->GetFile(MEM_HTML_NAME);

    std::ofstream oStream(memFile, std::ios::trunc);
    if (oStream.is_open()) {
        oStream << result;
        oStream.close();
    }
    _lock.unlock();
}


