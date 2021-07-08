//
// Created by Wande on 2/24/2021.
//

#ifndef D3PP_MEM_H
#define D3PP_MEM_H

#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <fstream>
#include "TaskScheduler.h"

const std::string MEM_HTML_NAME = "Mem_HTML";

struct MemElement {
    char* Memory;
    long Size;
    std::string File;
    int Line;
    std::string Message;
};

struct MemUsageChronic {
    long Mem;
    long Ram;
    long Page;
};

class Mem : TaskItem {
public:
    Mem();
    static char* Allocate(long size, std::string File, int line, std::string Message);
    static void Free(char* memory);
    static long MemoryUsage;
protected:
    static Mem* GetInstance();
    static Mem* singleton_;
    std::vector<MemElement> _elements;
private:
    int GetWorkingSetSize();
    int GetPageFileUsage();
    void MainFunc();
    void HtmlStats();


    static std::mutex _lock;

    std::vector<MemUsageChronic> _chronics;
};

const std::string MEM_HTML_TEMPLATE = R"(<html>
  <head>
    <title>Minecraft-Server Memory</title>
  </head>
  <body>
      <b><u>Overview:</u></b><br>
      Memory Usage (MEM):  [MEM]MB<br>
      Memory Usage (RAM):  [RAM]MB<br>
      Memory Usage (PAGE): [PGE]MB<br>
      Memory Usage (UNK):  [UNK]MB<br>
      Allocations: [ALLOCS]<br>
      <br>
      <b><u>Chronic:</u></b><br>
      <br>
      <table border=0 cellspacing=0>
        <tr>
         [CHRONIC_TABLE]
        </tr>
      </table>
      <br>
      <b><u>Fragmentation:</u></b><br>
      <br>
      <table border="0" cellspacing="0" cellpadding="0" bgcolor="#000000" >
        <tr>
        [FRAG_TABLE]
        </tr>
      </table>
      <br>
      <br>
      <br>
      <b><u>Elements:</u></b><br>
      <br>
      <table border=1>        <tr>
          <th><b>Address</b></th>
          <th><b>Size</b></th>
          <th><b>File</b></th>
          <th><b>Line</b></th>
          <th><b>Message</b></th>
        </tr>

        [ELEMENT_TABLE]
      </table>      <br>
      <br>
      <br>
      Site generated in [GEN_TIME] ms. [GEN_TIMESTAMP]<br>
  </body>
</html>)";

#endif //D3PP_MEM_H
