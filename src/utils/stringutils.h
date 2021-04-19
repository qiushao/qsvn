//
// Created by mint on 11/2/18.
//

#ifndef QTSVN_STRINGUTILS_H
#define QTSVN_STRINGUTILS_H

#include <string>
bool strEqual(std::string &str1, std::string &str2);
std::string strTrim(std::string &str);
std::string toLowerCase(std::string &str);
std::string toUpperCase(std::string &str);
bool isContainIgnoreCase(std::string &str, std::string &sub);


#endif //QTSVN_STRINGUTILS_H
