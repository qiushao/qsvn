//
// Created by mint on 11/1/18.
//

#ifndef QTSVN_SVNCLIENT_H
#define QTSVN_SVNCLIENT_H

#include <string>
#include <vector>

struct Path {
    std::string path;
    std::string kind;
    std::string action;
};

struct LogEntry {
    std::string revision;
    std::string author;
    std::string date;
    std::string msg;
    std::vector<Path*> pathList;
};

class SVNClient {
public:
    SVNClient();
    std::vector<LogEntry*> getLog(std::string uri,
            std::string startRevision,
            std::string endRevision,
            int limit);

    std::vector<Path*> getStatus(std::string uri);
    std::string commit(std::vector<Path*> pathList);
    std::string getRepositoryRoot(std::string uri);

private:
};

#endif //QTSVN_SVNCLIENT_H
