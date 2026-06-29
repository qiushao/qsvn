#pragma once

#include "SvnClient.h"

#include <QString>

struct ApplicationSettings {
    SvnOptions svn;
    QString externalDiffCommand;
    QString externalMergeCommand;

    static ApplicationSettings load();
    void save() const;
};
