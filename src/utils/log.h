//
// Created by qiushao on 18-4-21.
//

#ifndef __LOG_H__
#define __LOG_H__

#define LOG(...) log(__FILE__, __func__, __LINE__, __VA_ARGS__)
void log(const char* file, const char* func, const int line, const char *format, ...);
#endif //__LOG_H__
