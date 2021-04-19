//
// Created by qiushao on 18-4-21.
//

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "file.h"
#define PATH_MAX 4096


int cmd(const char *format, ...){
    char buffer[PATH_MAX];

    va_list arglist;
    va_start(arglist, format);
    vsprintf(buffer, format, arglist);
    va_end(arglist);

    printf("%s\n", buffer);
    return system(buffer);
}

std::string getExeFileDir() {
    char exePath[4096] = {0};
    readlink("/proc/self/exe", exePath, 4096);
    return dirName(exePath);
}
