//
// Created by mint on 11/2/18.
//

#include <algorithm>
#include <cstring>
#include "stringutils.h"

bool strEqual(std::string &str1, std::string &str2) {
    return (strcmp(str1.c_str(), str2.c_str()) == 0);
}

std::string strTrim(std::string &str) {
    const std::string drop = " ";
    // trim right
    str.erase(str.find_last_not_of(drop)+1);
    // trim left
    return str.erase(0, str.find_first_not_of(drop));
}

std::string toLowerCase(std::string &str) {
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))toupper);
    return s;
}

std::string toUpperCase(std::string &str) {
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))tolower);
    return s;
}

bool isContainIgnoreCase(std::string &str, std::string &sub) {
    std::string mstr = toLowerCase(str);
    std::string sstr = toLowerCase(sub);
    int index =  mstr.find(sstr);
    return index > -1;
}