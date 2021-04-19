//
// Created by qiushao on 18-4-21.
//

#include <stdarg.h>
#include <stdio.h>
#define PATH_MAX 4096


void log(const char* file, const char* func, const int line, const char *format, ...) {
    char buffer[PATH_MAX];
    char fmtBuffer[PATH_MAX];
    sprintf(fmtBuffer, "[%s:%d %s]:%s", file, line, func, format);

    va_list arglist;
    va_start(arglist, format);
    vsprintf(buffer, fmtBuffer, arglist);
    va_end(arglist);

    printf("%s\n", buffer);
}
