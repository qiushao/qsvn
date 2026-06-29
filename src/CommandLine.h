#pragma once

#include <QString>
#include <QStringList>

enum class CommandLineAction {
    None,
    Open,
    Status,
    Update,
    Commit,
    Add,
    Ignore,
    Copy,
    Delete,
    Rename,
    Revert,
    Cleanup,
    Diff,
    Log,
    Blame,
    Properties,
    Conflicts,
    RepoBrowser,
};

struct CommandLineRequest {
    CommandLineAction action = CommandLineAction::None;
    QString path;
    QString error;

    bool ok() const;
};

CommandLineRequest parseCommandLine(const QStringList &arguments);
