//
// Created by mint on 11/1/18.
//

#include <iostream>
#include <QtWidgets/QMessageBox>
#include "utils/cmd.h"
#include "utils/log.h"
#include "SVNClient.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"

SVNClient::SVNClient() {
}

static void dumpLogEntries(std::vector<LogEntry> &logList) {
    for (auto entry : logList) {
        std::cout << entry.revision << " | " << entry.author << " | " << entry.date << " | " << entry.msg <<std::endl;
    }
}



//svn log -l 100 -r 1845384:1 https://svn.apache.org/repos/asf/subversion/trunk
//按此方法可以实现 next 100 功能
std::vector<LogEntry*> SVNClient::getLog(std::string uri,
        std::string startRevision,
        std::string endRevision,
        int limit) {
    std::vector<LogEntry*> logList;
    char tmpPath[256];
    tmpnam(tmpPath);

    int ret = cmd("svn log -v -r %s:%s -l %d --xml %s > %s",
                  startRevision.c_str(), endRevision.c_str(), limit, uri.c_str(), tmpPath);
    if (ret != 0) {
        LOG("svn command error!, ret = %d", ret);
        QMessageBox::warning(NULL, "svn error", "svn command error, not a working copy?", QMessageBox::Yes, QMessageBox::Yes);
        return logList;
    }

    rapidxml::file<> fdoc(tmpPath);
    rapidxml::xml_document<> doc;
    doc.parse<0>(fdoc.data());
    rapidxml::xml_node<>* root = doc.first_node();
    rapidxml::xml_node<> *logNode;
    rapidxml::xml_node<> *pathsNode, *pathNode;
    for(logNode = root->first_node("logentry");logNode != nullptr;logNode = logNode->next_sibling())
    {
        auto *entry = new LogEntry();
        entry->revision = logNode->first_attribute()->value();
        entry->author = logNode->first_node("author")->value();
        entry->date = logNode->first_node("date")->value();
        entry->msg = logNode->first_node("msg")->value();

        pathsNode = logNode->first_node("paths");
        for (pathNode = pathsNode->first_node();  pathNode != nullptr; pathNode = pathNode->next_sibling()) {
            Path *path = new Path();
            path->kind = pathNode->first_attribute("kind")->value();
            path->action = pathNode->first_attribute("action")->value();
            path->path = pathNode->value();

            entry->pathList.push_back(path);
        }
        logList.push_back(entry);
    }

    cmd("rm -f %s", tmpPath);
    return logList;
}


std::vector<Path*> SVNClient::getStatus(std::string uri) {
    std::vector<Path*> pathList;
    char tmpPath[256];
    tmpnam(tmpPath);

    int ret = cmd("svn status --xml %s > %s", uri.c_str(), tmpPath);
    if (ret != 0) {
        LOG("svn command error!, ret = %d", ret);
        QMessageBox::warning(NULL, "svn error", "svn command error, not a working copy?", QMessageBox::Yes, QMessageBox::Yes);
        return pathList;
    }

    rapidxml::file<> fdoc(tmpPath);
    rapidxml::xml_document<> doc;
    doc.parse<0>(fdoc.data());
    rapidxml::xml_node<>* root = doc.first_node();
    //TODO

    return pathList;
}

std::string SVNClient::commit(std::vector<Path*> pathList) {

}

std::string SVNClient::getRepositoryRoot(std::string uri) {
    char tmpPath[256];
    tmpnam(tmpPath);

    int ret = cmd("svn info %s --xml > %s", uri.c_str(), tmpPath);
    if (ret != 0) {
        LOG("svn command error!, ret = %d", ret);
        return "";
    }

    rapidxml::file<> fdoc(tmpPath);
    rapidxml::xml_document<> doc;
    doc.parse<0>(fdoc.data());
    rapidxml::xml_node<>* root = doc.first_node("info")->first_node("entry")->first_node("repository")->first_node("root");

    std::string repoRoot = root->value();
    LOG("repo root = %s", repoRoot.c_str());
    cmd("rm -f %s", tmpPath);
    return root->value();
}