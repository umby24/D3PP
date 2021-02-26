//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_CHAT_H
#define D3PP_CHAT_H
#include <iostream>
#include <string>
#include <regex>

const std::regex AllowedRegexp("[^A-Za-z0-9!\\^\\~$%&/()=?{}\t\\[\\]\\\\ ,\\\";.:\\-_#'+*<>|@]|&.$|&.(&.)");
class Chat {
public:
    static std::string StringMultiline(std::string input);
    static bool StringIV(std::string input);
    static std::string StringGV(std::string input);
};


#endif //D3PP_CHAT_H
