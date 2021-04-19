//
// Created by qiushao on 18-4-21.
//

#ifndef __FILE_H__
#define __FILE_H__

#include <glob.h>
#include <string>
#include <vector>

bool isFleExist(const char* path);
size_t getFileSize(const char* path);
std::string dirName(const char* path);
std::string baseName(const char* path);
bool extractBlock(const char* inFile, const char* outFile, size_t startPos, size_t len);
bool extractBlockByFp(FILE *fp, const char* outFile, size_t len);
size_t getExt4BlockCount(const char* path);
size_t getExt4BlockSize(const char* path);
bool isSparseExt4File(const char* path);
bool isExt4File(const char* path);
void fixExt4File(const char* path);

uint16_t readUInt16(FILE *fp);
uint32_t readUInt32(FILE *fp);
uint64_t readUInt64(FILE *fp);

uint16_t readBEUInt16(FILE *fp);
uint32_t readBEUInt32(FILE *fp);
uint64_t readBEUInt64(FILE *fp);

void writeBEUInt16(FILE *fp, uint16_t data);
void writeBEUInt32(FILE *fp, uint32_t data);
void writeBEUInt64(FILE *fp, uint64_t data);

void writeFile(FILE *fp, const char *file);

std::string readFileIntoString(const char *path);
void filterFiles(std::vector<std::string> &fileList, const char *dirPath, const char *pattern);
#endif //__FILE_H__
