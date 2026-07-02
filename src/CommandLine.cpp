#include "CommandLine.h"

bool CommandLineRequest::ok() const
{
    return error.isEmpty();
}

CommandLineRequest parseCommandLine(const QStringList &arguments)
{
    CommandLineRequest request;
    if (arguments.size() <= 1) {
        return request;
    }

    auto readPath = [&arguments, &request](int index) {
        if (index + 1 >= arguments.size()) {
            request.error = QStringLiteral("Missing path argument.");
            return QString();
        }
        return arguments.at(index + 1);
    };

    const QString command = arguments.at(1);
    if (command == QStringLiteral("--repo-browser")) {
        request.action = CommandLineAction::RepoBrowser;
        if (arguments.size() > 2) {
            request.path = arguments.at(2);
        }
        return request;
    }
    if (command == QStringLiteral("--settings")) {
        request.action = CommandLineAction::Settings;
        return request;
    }
    if (command == QStringLiteral("--open")) {
        request.action = CommandLineAction::Open;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--status")) {
        request.action = CommandLineAction::Status;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--checkout")) {
        request.action = CommandLineAction::Checkout;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--create-repository")) {
        request.action = CommandLineAction::CreateRepository;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--check-repository")) {
        request.action = CommandLineAction::CheckRepository;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--export")) {
        request.action = CommandLineAction::Export;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--import")) {
        request.action = CommandLineAction::Import;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--branch-tag")) {
        request.action = CommandLineAction::BranchTag;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--switch")) {
        request.action = CommandLineAction::Switch;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--relocate")) {
        request.action = CommandLineAction::Relocate;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--merge")) {
        request.action = CommandLineAction::Merge;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--update")) {
        request.action = CommandLineAction::Update;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--update-revision")) {
        request.action = CommandLineAction::UpdateRevision;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--commit")) {
        request.action = CommandLineAction::Commit;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--add")) {
        request.action = CommandLineAction::Add;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--ignore")) {
        request.action = CommandLineAction::Ignore;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--copy")) {
        request.action = CommandLineAction::Copy;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--delete")) {
        request.action = CommandLineAction::Delete;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--rename")) {
        request.action = CommandLineAction::Rename;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--revert")) {
        request.action = CommandLineAction::Revert;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--set-changelist")) {
        request.action = CommandLineAction::SetChangelist;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--remove-changelist")) {
        request.action = CommandLineAction::RemoveChangelist;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--resolve")) {
        request.action = CommandLineAction::Resolve;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--lock")) {
        request.action = CommandLineAction::Lock;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--unlock")) {
        request.action = CommandLineAction::Unlock;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--cleanup")) {
        request.action = CommandLineAction::Cleanup;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--diff")) {
        request.action = CommandLineAction::Diff;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--create-patch")) {
        request.action = CommandLineAction::CreatePatch;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--apply-patch")) {
        request.action = CommandLineAction::ApplyPatch;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--log")) {
        request.action = CommandLineAction::Log;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--blame")) {
        request.action = CommandLineAction::Blame;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--properties")) {
        request.action = CommandLineAction::Properties;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--conflicts")) {
        request.action = CommandLineAction::Conflicts;
        request.path = readPath(1);
        return request;
    }
    if (command == QStringLiteral("--edit-conflict")) {
        request.action = CommandLineAction::EditConflict;
        request.path = readPath(1);
        return request;
    }
    if (command.startsWith(QStringLiteral("--"))) {
        request.error = QStringLiteral("Unknown option: ") + command;
        return request;
    }

    request.action = CommandLineAction::Open;
    request.path = command;
    return request;
}
