#include "CommandLine.h"
#include "MainWindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qsvn"));
    QApplication::setOrganizationName(QStringLiteral("qsvn"));

    const CommandLineRequest commandLine = parseCommandLine(QCoreApplication::arguments());

    MainWindow window;
    window.showMaximized();

    if (!commandLine.ok()) {
        QTimer::singleShot(0, &window, [&window, commandLine]() {
            QMessageBox::warning(&window, QStringLiteral("qsvn"), commandLine.error);
        });
    } else {
        QTimer::singleShot(0, &window, [&window, commandLine]() {
            switch (commandLine.action) {
            case CommandLineAction::None:
                break;
            case CommandLineAction::Open:
            case CommandLineAction::Status:
                window.openPath(commandLine.path);
                break;
            case CommandLineAction::CheckRepository:
                window.checkRepositoryPath(commandLine.path);
                break;
            case CommandLineAction::Export:
                window.exportPath(commandLine.path);
                break;
            case CommandLineAction::Import:
                window.importPath(commandLine.path);
                break;
            case CommandLineAction::Update:
                window.updatePath(commandLine.path);
                break;
            case CommandLineAction::UpdateRevision:
                window.updateToRevisionPath(commandLine.path);
                break;
            case CommandLineAction::Commit:
                window.commitPath(commandLine.path);
                break;
            case CommandLineAction::Add:
                window.addPath(commandLine.path);
                break;
            case CommandLineAction::Ignore:
                window.ignorePath(commandLine.path);
                break;
            case CommandLineAction::Copy:
                window.copyPath(commandLine.path);
                break;
            case CommandLineAction::Delete:
                window.deletePath(commandLine.path);
                break;
            case CommandLineAction::Rename:
                window.renamePath(commandLine.path);
                break;
            case CommandLineAction::Revert:
                window.revertPath(commandLine.path);
                break;
            case CommandLineAction::Lock:
                window.lockPath(commandLine.path);
                break;
            case CommandLineAction::Unlock:
                window.unlockPath(commandLine.path);
                break;
            case CommandLineAction::Cleanup:
                window.cleanupPath(commandLine.path);
                break;
            case CommandLineAction::Diff:
                window.showDiffForPath(commandLine.path);
                break;
            case CommandLineAction::Log:
                window.showLogForPath(commandLine.path);
                break;
            case CommandLineAction::Blame:
                window.showBlameForPath(commandLine.path);
                break;
            case CommandLineAction::Properties:
                window.showPropertiesForPath(commandLine.path);
                break;
            case CommandLineAction::Conflicts:
                window.showConflictsForPath(commandLine.path);
                break;
            case CommandLineAction::RepoBrowser:
                window.showRepositoryBrowser();
                break;
            }
        });
    }

    return app.exec();
}
