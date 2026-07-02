#pragma once

#include <QString>
#include <QStringList>

enum class CommandLineAction {
    None,
    Open,
    Settings,
    Status,
    Checkout,
    CreateRepository,
    CheckRepository,
    Export,
    Import,
    BranchTag,
    Switch,
    Relocate,
    Merge,
    Update,
    UpdateRevision,
    Commit,
    Add,
    Ignore,
    Copy,
    Delete,
    Rename,
    Revert,
    SetChangelist,
    RemoveChangelist,
    Resolve,
    Lock,
    Unlock,
    Cleanup,
    Diff,
    CreatePatch,
    ApplyPatch,
    Log,
    Blame,
    Properties,
    Conflicts,
    EditConflict,
    RepoBrowser,
};

struct CommandLineRequest {
    CommandLineAction action = CommandLineAction::None;
    QString path;
    QString error;

    bool ok() const;
};

CommandLineRequest parseCommandLine(const QStringList &arguments);
