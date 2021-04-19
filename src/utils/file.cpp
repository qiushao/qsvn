//
// Created by qiushao on 18-4-21.
//
#include <string>
#include <stddef.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>
#include "log.h"

using namespace std;

bool isFleExist(const char *path) {
    return (access(path, 0) != -1);
}

size_t getFileSize(const char *path) {
    struct stat st;
    stat(path, &st);
    return (size_t) st.st_size;
}

string dirName(const char *path) {
    char buffer[PATH_MAX];
    strcpy(buffer, path);
    char *name = dirname(buffer);
    return string(name);
}

string baseName(const char *path) {
    char buffer[PATH_MAX];
    strcpy(buffer, path);
    char *name = basename(buffer);
    return string(name);
}

bool extractBlock(const char *inFile, const char *outFile, size_t startPos, size_t len) {
    FILE *input = fopen(inFile, "rb");
    FILE *output = fopen(outFile, "wb");

    char buff[4096];
    size_t buffsize = 4096;
    size_t offset = startPos, size = len, read, tmp;

    fseek(input, offset, SEEK_SET);
    while (size > 0) {
        tmp = size > buffsize ? buffsize : size;
        read = fread(buff, 1, tmp, input);
        if (read <= 0) {
            printf("read <=0, exit");
            fclose(input);
            fclose(output);
            return false;
        }
        fwrite(buff, read, 1, output);
        size -= read;
    }
    fclose(output);
    fclose(input);

    return true;
}

bool extractBlockByFp(FILE *fp, const char *outFile, size_t len) {
    FILE *output = fopen(outFile, "wb");

    char buff[4096];
    size_t buffsize = 4096;
    size_t size = len, read, tmp;

    while (size > 0) {
        tmp = size > buffsize ? buffsize : size;
        read = fread(buff, 1, tmp, fp);
        if (read <= 0) {
            printf("read <=0, exit");
            fclose(output);
            return false;
        }
        fwrite(buff, read, 1, output);
        size -= read;
    }
    fclose(output);

    return true;
}

bool isExt4File(const char *path) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0x438, SEEK_SET);
    uint16_t magic;
    fread(&magic, sizeof(magic), 1, fp);
    LOG("magic = %x", magic);
    fclose(fp);

    return magic == 0xEF53;
}

bool isSparseExt4File(const char *path) {
    FILE *fp = fopen(path, "rb");
    uint32_t magic;
    fread(&magic, sizeof(magic), 1, fp);
    LOG("magic = %x", magic);
    fclose(fp);

    return magic == 0xED26FF3A;
}


size_t getExt4BlockCount(const char *path) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0x404, SEEK_SET);
    uint32_t count;
    fread(&count, sizeof(count), 1, fp);
    fclose(fp);
    LOG("block clount = %x", count);
    return (size_t) count;
}

size_t getExt4BlockSize(const char *path) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0x418, SEEK_SET);
    uint32_t size;
    fread(&size, sizeof(size), 1, fp);
    fclose(fp);
    size = (1024 << size);
    LOG("block size = %x", size);
    return (size_t) size;
}

void fixExt4File(const char *path) {
    LOG("fixExt4File start...");
    size_t realSize = getExt4BlockCount(path) * getExt4BlockSize(path);
    size_t curSize = getFileSize(path);
    size_t restSize = realSize - curSize;

    LOG("realSize = %zd, curSize = %zd, restSize = %zd", realSize, curSize, restSize);
    if (restSize <= 0) {
        return;
    }

    const uint32_t buffsize = 8192; //8k
    char buffer[buffsize];
    memset(buffer, 0, buffsize);

    FILE *fp = fopen(path, "a+b");

    while (restSize >= buffsize) {
        fwrite(buffer, buffsize, 1, fp);
        restSize -= buffsize;
    }

    fwrite(buffer, restSize, 1, fp);
    fclose(fp);
}

uint16_t readUInt16(FILE *fp) {
    uint16_t value;
    fread(&value, sizeof(value), 1, fp);
    return value;
}

uint32_t readUInt32(FILE *fp) {
    uint32_t value;
    fread(&value, sizeof(value), 1, fp);
    return value;
}

uint64_t readUInt64(FILE *fp) {
    uint64_t value;
    fread(&value, sizeof(value), 1, fp);
    return value;
}

uint16_t readBEUInt16(FILE *fp) {
    uint16_t value;
    fread(&value, sizeof(value), 1, fp);
    return htobe16(value);
}

uint32_t readBEUInt32(FILE *fp) {
    uint32_t value;
    fread(&value, sizeof(value), 1, fp);
    return htobe32(value);
}

uint64_t readBEUInt64(FILE *fp) {
    uint64_t value;
    fread(&value, sizeof(value), 1, fp);
    return htobe64(value);
}

void writeBEUInt16(FILE *fp, uint16_t data) {
    uint16_t value = htobe16(data);
    fwrite(&value, sizeof(value), 1, fp);
}

void writeBEUInt32(FILE *fp, uint32_t data) {
    uint32_t value = htobe32(data);
    fwrite(&value, sizeof(value), 1, fp);
}

void writeBEUInt64(FILE *fp, uint64_t data) {
    uint64_t value = htobe64(data);
    fwrite(&value, sizeof(value), 1, fp);
}

void writeFile(FILE *fp, const char *file) {
    size_t length = getFileSize(file);
    char *buffer = static_cast<char *>(malloc(length));
    FILE *in = fopen(file, "r");
    fread(buffer, length, 1, in);
    fclose(in);
    fwrite(buffer, length, 1, fp);
    free(buffer);
}

string readFileIntoString(const char *path) {
    ifstream ifile(path);
    ostringstream buf;
    char ch;
    while (buf && ifile.get(ch)) {
        buf.put(ch);
    }
    return buf.str();
}

void filterFiles(vector<string> &fileList, const char *dirPath, const char *pattern) {
    DIR *dir;
    struct dirent *entry;
    int ret;

    dir = opendir(dirPath); //打开指定路径

    //路径存在
    if(dir != NULL)
    {
        //逐个获取文件夹中文件
        while( (entry = readdir(dir)) != NULL)
        {
//            LOG("find entry %s", entry->d_name);
            ret = fnmatch(pattern, entry->d_name, FNM_PATHNAME|FNM_PERIOD);
            if(ret == 0)          //符合pattern的结构
            {
//                LOG("%s\n", entry->d_name);
                fileList.insert(fileList.end(), entry->d_name);
            }else if(ret == FNM_NOMATCH){
                continue ;
            }else
            {
                LOG("error file=%s\n", entry->d_name);
            }
        }
        closedir(dir);
    } else {
        LOG("can't open dir %s", dirPath);
    }

}