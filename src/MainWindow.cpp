#include "MainWindow.h"

#include "BranchTagDialog.h"
#include "CheckoutDialog.h"
#include "CommitDialog.h"
#include "ConflictsDialog.h"
#include "ExportDialog.h"
#include "ExternalToolCommand.h"
#include "ImportDialog.h"
#include "LogDialog.h"
#include "MergeDialog.h"
#include "PropertiesDialog.h"
#include "RepositoryBrowserDialog.h"
#include "RelocateDialog.h"
#include "SettingsDialog.h"
#include "SwitchDialog.h"

#include <algorithm>

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QStandardPaths>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("qsvn"));
    resize(1180, 760);
    m_settings = ApplicationSettings::load();
    m_svn.setOptions(m_settings.svn);

    createActions();
    createCentralWidget();
    createMenus();
    createToolBar();
    updateActionState();
    statusBar()->showMessage(QStringLiteral("Open or checkout a working copy."));
}

bool MainWindow::openPath(const QString &path)
{
    const QString root = workingCopyRootForPath(path);
    if (root.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Open"), QStringLiteral("The selected path is not an SVN working copy path."));
        return false;
    }

    setWorkingCopy(root);
    return true;
}

bool MainWindow::updatePath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("update"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Update failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::commitPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult statusResult = m_svn.run({QStringLiteral("status"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(statusResult);

    if (!statusResult.ok()) {
        QMessageBox::warning(this, QStringLiteral("Commit"), statusResult.combinedOutput());
        return false;
    }

    const QVector<SvnStatus> statuses = m_svn.parseStatus(statusResult.standardOutput);
    CommitDialog dialog(statuses, this);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    QStringList arguments = {QStringLiteral("commit"), QStringLiteral("-m"), dialog.message()};
    arguments.append(dialog.selectedPaths());
    runWorkingCopyCommand(arguments, true);
    return true;
}

bool MainWindow::addPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("add"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Add failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::ignorePath(const QString &path)
{
    const QFileInfo targetInfo(path);
    const QString targetName = targetInfo.fileName();
    const QString parentPath = targetInfo.absolutePath();
    if (targetName.isEmpty() || parentPath.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Ignore"), QStringLiteral("Select a file or folder to ignore."));
        return false;
    }

    const QString root = workingCopyRootForPath(parentPath);
    if (root.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Ignore"), QStringLiteral("The selected path's parent is not an SVN working copy path."));
        return false;
    }

    setWorkingCopy(root);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult propgetResult = m_svn.run({QStringLiteral("propget"), QStringLiteral("svn:ignore"), parentPath}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(propgetResult);

    if (!propgetResult.ok()) {
        QMessageBox::warning(this, QStringLiteral("Ignore failed"), propgetResult.combinedOutput());
        return false;
    }

    QStringList ignoredNames;
    const QStringList lines = propgetResult.standardOutput.split('\n');
    for (QString line : lines) {
        if (line.endsWith('\r')) {
            line.chop(1);
        }
        if (!line.isEmpty()) {
            ignoredNames.push_back(line);
        }
    }

    if (ignoredNames.contains(targetName)) {
        QMessageBox::information(this, QStringLiteral("Ignore"), QStringLiteral("'%1' is already ignored.").arg(targetName));
        refreshStatus();
        return true;
    }

    ignoredNames.push_back(targetName);
    const QString propertyValue = ignoredNames.join('\n') + '\n';

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("propset"), QStringLiteral("svn:ignore"), propertyValue, parentPath}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Ignore failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::copyPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    const QFileInfo sourceInfo(path);
    const QString oldName = sourceInfo.fileName();
    if (oldName.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Copy"), QStringLiteral("Select a file or folder to copy."));
        return false;
    }

    bool ok = false;
    const QString newName = QInputDialog::getText(this,
                                                  QStringLiteral("Copy"),
                                                  QStringLiteral("New copy name:"),
                                                  QLineEdit::Normal,
                                                  QStringLiteral("Copy of ") + oldName,
                                                  &ok)
                                .trimmed();
    if (!ok || newName.isEmpty() || newName == oldName) {
        return false;
    }
    if (newName.contains('/') || newName.contains('\\')) {
        QMessageBox::warning(this, QStringLiteral("Copy"), QStringLiteral("Enter a name, not a path."));
        return false;
    }

    const QString targetPath = QDir(sourceInfo.absolutePath()).absoluteFilePath(newName);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("copy"), sourceInfo.absoluteFilePath(), targetPath}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Copy failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::deletePath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    if (QMessageBox::question(this, QStringLiteral("Delete"), QStringLiteral("Delete '%1' with SVN?").arg(path)) != QMessageBox::Yes) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("delete"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Delete failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::renamePath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    const QFileInfo sourceInfo(path);
    const QString oldName = sourceInfo.fileName();
    if (oldName.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Rename"), QStringLiteral("Select a file or folder to rename."));
        return false;
    }

    bool ok = false;
    const QString newName = QInputDialog::getText(this,
                                                  QStringLiteral("Rename"),
                                                  QStringLiteral("New name:"),
                                                  QLineEdit::Normal,
                                                  oldName,
                                                  &ok)
                                .trimmed();
    if (!ok || newName.isEmpty() || newName == oldName) {
        return false;
    }
    if (newName.contains('/') || newName.contains('\\')) {
        QMessageBox::warning(this, QStringLiteral("Rename"), QStringLiteral("Enter a name, not a path."));
        return false;
    }

    const QString targetPath = QDir(sourceInfo.absolutePath()).absoluteFilePath(newName);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("move"), sourceInfo.absoluteFilePath(), targetPath}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Rename failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::revertPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    if (QMessageBox::question(this, QStringLiteral("Revert"), QStringLiteral("Revert '%1'?").arg(path)) != QMessageBox::Yes) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("revert"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Revert failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::cleanupPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("cleanup"), m_workingCopy}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Cleanup failed"), result.combinedOutput());
        return false;
    }

    refreshStatus();
    return true;
}

bool MainWindow::showDiffForPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("diff"), QStringLiteral("--internal-diff"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Diff failed"), result.combinedOutput());
        return false;
    }

    showTextDialog(QStringLiteral("Diff"), result.standardOutput);
    return true;
}

bool MainWindow::showLogForPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("log"), QStringLiteral("--xml"), QStringLiteral("-v"), QStringLiteral("-l"), QStringLiteral("100"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Log failed"), result.combinedOutput());
        return false;
    }

    const QVector<SvnLogEntry> entries = m_svn.parseLogXml(result.standardOutput);
    if (entries.isEmpty() && !result.standardOutput.trimmed().isEmpty()) {
        showTextDialog(QStringLiteral("Log"), result.standardOutput);
        return true;
    }

    auto *dialog = new LogDialog(entries, this);
    connect(dialog, &LogDialog::revisionDiffRequested, this, [this, path](const QString &revision) {
        QStringList diffArguments = {QStringLiteral("diff"), QStringLiteral("--internal-diff"), QStringLiteral("-c"), revision, path};

        QApplication::setOverrideCursor(Qt::WaitCursor);
        const SvnResult diffResult = m_svn.run(diffArguments, m_workingCopy);
        QApplication::restoreOverrideCursor();
        appendResult(diffResult);

        if (!diffResult.ok()) {
            QMessageBox::warning(this, QStringLiteral("Revision diff failed"), diffResult.combinedOutput());
            return;
        }

        showTextDialog(QStringLiteral("Revision ") + revision + QStringLiteral(" Diff"), diffResult.standardOutput);
    });
    connect(dialog, &LogDialog::revisionBlameRequested, this, [this](const QString &revision, const QString &repositoryPath) {
        QString blameTarget = repositoryPath;
        if (repositoryPath.startsWith('/')) {
            const QString rootUrl = workingCopyRepositoryRootUrl();
            if (!rootUrl.isEmpty()) {
                blameTarget = rootUrl + repositoryPath;
            }
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        const SvnResult blameResult = m_svn.run({QStringLiteral("blame"), QStringLiteral("-r"), revision, blameTarget}, m_workingCopy);
        QApplication::restoreOverrideCursor();
        appendResult(blameResult);

        if (!blameResult.ok()) {
            QMessageBox::warning(this, QStringLiteral("Blame failed"), blameResult.combinedOutput());
            return;
        }

        showTextDialog(QStringLiteral("Blame r") + revision, blameResult.standardOutput);
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    return true;
}

bool MainWindow::showBlameForPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("blame"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Blame failed"), result.combinedOutput());
        return false;
    }

    showTextDialog(QStringLiteral("Blame"), result.standardOutput);
    return true;
}

bool MainWindow::showPropertiesForPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    auto *dialog = new PropertiesDialog(&m_svn, m_workingCopy, QFileInfo(path).absoluteFilePath(), this);
    connect(dialog, &PropertiesDialog::commandFinished, this, &MainWindow::appendResult);
    connect(dialog, &QDialog::finished, this, [this]() {
        refreshStatus();
    });
    dialog->refresh();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    return true;
}

bool MainWindow::showConflictsForPath(const QString &path)
{
    if (!openPath(path)) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("status"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Conflicts"), result.combinedOutput());
        return false;
    }

    QVector<SvnStatus> conflicts;
    const QVector<SvnStatus> statuses = m_svn.parseStatus(result.standardOutput);
    for (const SvnStatus &status : statuses) {
        if (status.code.contains('C')) {
            conflicts.push_back(status);
        }
    }

    if (conflicts.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Conflicts"), QStringLiteral("No conflicts were found."));
        return true;
    }

    auto *dialog = new ConflictsDialog(conflicts, m_workingCopy, this);
    connect(dialog, &ConflictsDialog::editConflictRequested, this, [this](const QString &conflictPath) {
        launchExternalMerge(conflictPath);
    });
    connect(dialog, &ConflictsDialog::resolveRequested, this, [this, dialog](const QString &conflictPath, const QString &acceptChoice) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        const SvnResult resolveResult = m_svn.run({QStringLiteral("resolve"), QStringLiteral("--accept"), acceptChoice, conflictPath}, m_workingCopy);
        QApplication::restoreOverrideCursor();
        appendResult(resolveResult);

        if (!resolveResult.ok()) {
            QMessageBox::warning(this, QStringLiteral("Resolve failed"), resolveResult.combinedOutput());
            return;
        }

        refreshStatus();
        dialog->accept();
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    return true;
}

void MainWindow::showRepositoryBrowser()
{
    browseRepository();
}

void MainWindow::createActions()
{
    m_openAction = new QAction(style()->standardIcon(QStyle::SP_DirOpenIcon), QStringLiteral("Open"), this);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openWorkingCopy);

    m_settingsAction = new QAction(QStringLiteral("Settings"), this);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::openSettings);

    m_checkoutAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), QStringLiteral("Checkout"), this);
    connect(m_checkoutAction, &QAction::triggered, this, &MainWindow::checkout);

    m_exportAction = new QAction(QStringLiteral("Export"), this);
    connect(m_exportAction, &QAction::triggered, this, &MainWindow::exportProject);

    m_importAction = new QAction(QStringLiteral("Import"), this);
    connect(m_importAction, &QAction::triggered, this, &MainWindow::importProject);

    m_repoBrowserAction = new QAction(QStringLiteral("Repo Browser"), this);
    connect(m_repoBrowserAction, &QAction::triggered, this, &MainWindow::browseRepository);

    m_branchTagAction = new QAction(QStringLiteral("Branch / Tag"), this);
    connect(m_branchTagAction, &QAction::triggered, this, &MainWindow::branchOrTag);

    m_switchAction = new QAction(QStringLiteral("Switch"), this);
    connect(m_switchAction, &QAction::triggered, this, &MainWindow::switchWorkingCopy);

    m_relocateAction = new QAction(QStringLiteral("Relocate"), this);
    connect(m_relocateAction, &QAction::triggered, this, &MainWindow::relocateWorkingCopy);

    m_mergeAction = new QAction(QStringLiteral("Merge"), this);
    connect(m_mergeAction, &QAction::triggered, this, &MainWindow::mergeIntoWorkingCopy);

    m_refreshAction = new QAction(style()->standardIcon(QStyle::SP_BrowserReload), QStringLiteral("Status"), this);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshStatus);

    m_updateAction = new QAction(style()->standardIcon(QStyle::SP_ArrowDown), QStringLiteral("Update"), this);
    connect(m_updateAction, &QAction::triggered, this, &MainWindow::updateWorkingCopy);

    m_updateRevisionAction = new QAction(QStringLiteral("Update to Revision"), this);
    connect(m_updateRevisionAction, &QAction::triggered, this, &MainWindow::updateToRevision);

    m_commitAction = new QAction(style()->standardIcon(QStyle::SP_DialogApplyButton), QStringLiteral("Commit"), this);
    connect(m_commitAction, &QAction::triggered, this, &MainWindow::commitChanges);

    m_addAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), QStringLiteral("Add"), this);
    connect(m_addAction, &QAction::triggered, this, &MainWindow::addSelected);

    m_ignoreAction = new QAction(QStringLiteral("Ignore"), this);
    connect(m_ignoreAction, &QAction::triggered, this, &MainWindow::ignoreSelected);

    m_copyAction = new QAction(QStringLiteral("Copy"), this);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copySelected);

    m_deleteAction = new QAction(style()->standardIcon(QStyle::SP_TrashIcon), QStringLiteral("Delete"), this);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::deleteSelected);

    m_renameAction = new QAction(QStringLiteral("Rename"), this);
    connect(m_renameAction, &QAction::triggered, this, &MainWindow::renameSelected);

    m_revertAction = new QAction(style()->standardIcon(QStyle::SP_DialogCancelButton), QStringLiteral("Revert"), this);
    connect(m_revertAction, &QAction::triggered, this, &MainWindow::revertSelected);

    m_setChangelistAction = new QAction(QStringLiteral("Set Changelist"), this);
    connect(m_setChangelistAction, &QAction::triggered, this, &MainWindow::setChangelist);

    m_removeChangelistAction = new QAction(QStringLiteral("Remove Changelist"), this);
    connect(m_removeChangelistAction, &QAction::triggered, this, &MainWindow::removeChangelist);

    m_cleanupAction = new QAction(QStringLiteral("Cleanup"), this);
    connect(m_cleanupAction, &QAction::triggered, this, &MainWindow::cleanupWorkingCopy);

    m_resolveAction = new QAction(QStringLiteral("Resolve"), this);
    connect(m_resolveAction, &QAction::triggered, this, &MainWindow::resolveSelected);

    m_conflictsAction = new QAction(QStringLiteral("Conflicts"), this);
    connect(m_conflictsAction, &QAction::triggered, this, &MainWindow::showConflicts);

    m_editConflictAction = new QAction(QStringLiteral("Edit Conflict"), this);
    connect(m_editConflictAction, &QAction::triggered, this, &MainWindow::editConflict);

    m_lockAction = new QAction(QStringLiteral("Lock"), this);
    connect(m_lockAction, &QAction::triggered, this, &MainWindow::lockSelected);

    m_unlockAction = new QAction(QStringLiteral("Unlock"), this);
    connect(m_unlockAction, &QAction::triggered, this, &MainWindow::unlockSelected);

    m_diffAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), QStringLiteral("Diff"), this);
    connect(m_diffAction, &QAction::triggered, this, &MainWindow::diffSelected);

    m_createPatchAction = new QAction(QStringLiteral("Create Patch"), this);
    connect(m_createPatchAction, &QAction::triggered, this, &MainWindow::createPatch);

    m_applyPatchAction = new QAction(QStringLiteral("Apply Patch"), this);
    connect(m_applyPatchAction, &QAction::triggered, this, &MainWindow::applyPatch);

    m_propertiesAction = new QAction(QStringLiteral("Properties"), this);
    connect(m_propertiesAction, &QAction::triggered, this, &MainWindow::openProperties);

    m_logAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), QStringLiteral("Log"), this);
    connect(m_logAction, &QAction::triggered, this, &MainWindow::logSelected);

    m_blameAction = new QAction(QStringLiteral("Blame"), this);
    connect(m_blameAction, &QAction::triggered, this, &MainWindow::blameSelected);
}

void MainWindow::createCentralWidget()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);

    m_pathEdit = new QLineEdit(central);
    m_pathEdit->setReadOnly(true);
    m_pathEdit->setPlaceholderText(QStringLiteral("No working copy opened"));
    layout->addWidget(m_pathEdit);

    auto *mainSplitter = new QSplitter(Qt::Horizontal, central);
    layout->addWidget(mainSplitter, 1);

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_fileModel->setRootPath(QDir::homePath());

    m_treeView = new QTreeView(mainSplitter);
    m_treeView->setModel(m_fileModel);
    m_treeView->setRootIndex(m_fileModel->index(QDir::homePath()));
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    for (int column = 1; column < m_fileModel->columnCount(); ++column) {
        m_treeView->hideColumn(column);
    }
    connect(m_treeView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &position) {
        QMenu menu(this);
        menu.addAction(m_addAction);
        menu.addAction(m_ignoreAction);
        menu.addAction(m_copyAction);
        menu.addAction(m_deleteAction);
        menu.addAction(m_renameAction);
        menu.addAction(m_revertAction);
        menu.addAction(m_setChangelistAction);
        menu.addAction(m_removeChangelistAction);
        menu.addAction(m_resolveAction);
        menu.addAction(m_conflictsAction);
        menu.addAction(m_editConflictAction);
        menu.addSeparator();
        menu.addAction(m_lockAction);
        menu.addAction(m_unlockAction);
        menu.addSeparator();
        menu.addAction(m_diffAction);
        menu.addAction(m_propertiesAction);
        menu.addAction(m_logAction);
        menu.addAction(m_blameAction);
        menu.exec(m_treeView->viewport()->mapToGlobal(position));
    });

    auto *rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);

    m_statusTable = new QTableWidget(rightSplitter);
    m_statusTable->setColumnCount(2);
    m_statusTable->setHorizontalHeaderLabels({QStringLiteral("Status"), QStringLiteral("Path")});
    m_statusTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_statusTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_statusTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_statusTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_statusTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statusTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_statusTable, &QTableWidget::customContextMenuRequested, this, [this](const QPoint &position) {
        QMenu menu(this);
        menu.addAction(m_addAction);
        menu.addAction(m_ignoreAction);
        menu.addAction(m_copyAction);
        menu.addAction(m_deleteAction);
        menu.addAction(m_renameAction);
        menu.addAction(m_revertAction);
        menu.addAction(m_setChangelistAction);
        menu.addAction(m_removeChangelistAction);
        menu.addAction(m_resolveAction);
        menu.addAction(m_conflictsAction);
        menu.addAction(m_editConflictAction);
        menu.addSeparator();
        menu.addAction(m_lockAction);
        menu.addAction(m_unlockAction);
        menu.addSeparator();
        menu.addAction(m_diffAction);
        menu.addAction(m_propertiesAction);
        menu.addAction(m_logAction);
        menu.addAction(m_blameAction);
        menu.exec(m_statusTable->viewport()->mapToGlobal(position));
    });

    m_outputView = new QPlainTextEdit(rightSplitter);
    m_outputView->setReadOnly(true);
    m_outputView->setPlaceholderText(QStringLiteral("SVN command output"));

    rightSplitter->setStretchFactor(0, 3);
    rightSplitter->setStretchFactor(1, 2);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 2);

    setCentralWidget(central);
}

void MainWindow::createMenus()
{
    auto *repositoryMenu = menuBar()->addMenu(QStringLiteral("Repository"));
    repositoryMenu->addAction(m_openAction);
    repositoryMenu->addAction(m_checkoutAction);
    repositoryMenu->addAction(m_exportAction);
    repositoryMenu->addAction(m_importAction);
    repositoryMenu->addAction(m_repoBrowserAction);
    repositoryMenu->addSeparator();
    repositoryMenu->addAction(m_branchTagAction);
    repositoryMenu->addAction(m_switchAction);
    repositoryMenu->addAction(m_relocateAction);
    repositoryMenu->addAction(m_mergeAction);
    repositoryMenu->addSeparator();
    repositoryMenu->addAction(m_refreshAction);
    repositoryMenu->addAction(m_updateAction);
    repositoryMenu->addAction(m_updateRevisionAction);
    repositoryMenu->addAction(m_commitAction);

    auto *workingCopyMenu = menuBar()->addMenu(QStringLiteral("Working Copy"));
    workingCopyMenu->addAction(m_addAction);
    workingCopyMenu->addAction(m_ignoreAction);
    workingCopyMenu->addAction(m_copyAction);
    workingCopyMenu->addAction(m_deleteAction);
    workingCopyMenu->addAction(m_renameAction);
    workingCopyMenu->addAction(m_revertAction);
    workingCopyMenu->addAction(m_setChangelistAction);
    workingCopyMenu->addAction(m_removeChangelistAction);
    workingCopyMenu->addAction(m_resolveAction);
    workingCopyMenu->addAction(m_conflictsAction);
    workingCopyMenu->addAction(m_editConflictAction);
    workingCopyMenu->addSeparator();
    workingCopyMenu->addAction(m_cleanupAction);
    workingCopyMenu->addAction(m_lockAction);
    workingCopyMenu->addAction(m_unlockAction);
    workingCopyMenu->addSeparator();
    workingCopyMenu->addAction(m_diffAction);
    workingCopyMenu->addAction(m_createPatchAction);
    workingCopyMenu->addAction(m_applyPatchAction);
    workingCopyMenu->addAction(m_propertiesAction);
    workingCopyMenu->addAction(m_logAction);
    workingCopyMenu->addAction(m_blameAction);

    auto *toolsMenu = menuBar()->addMenu(QStringLiteral("Tools"));
    toolsMenu->addAction(m_settingsAction);
}

void MainWindow::createToolBar()
{
    auto *toolbar = addToolBar(QStringLiteral("SVN"));
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(m_openAction);
    toolbar->addAction(m_settingsAction);
    toolbar->addSeparator();
    toolbar->addAction(m_checkoutAction);
    toolbar->addAction(m_exportAction);
    toolbar->addAction(m_repoBrowserAction);
    toolbar->addSeparator();
    toolbar->addAction(m_branchTagAction);
    toolbar->addAction(m_switchAction);
    toolbar->addAction(m_relocateAction);
    toolbar->addAction(m_mergeAction);
    toolbar->addSeparator();
    toolbar->addAction(m_refreshAction);
    toolbar->addAction(m_updateAction);
    toolbar->addAction(m_commitAction);
    toolbar->addSeparator();
    toolbar->addAction(m_addAction);
    toolbar->addAction(m_ignoreAction);
    toolbar->addAction(m_copyAction);
    toolbar->addAction(m_deleteAction);
    toolbar->addAction(m_renameAction);
    toolbar->addAction(m_revertAction);
    toolbar->addAction(m_diffAction);
    toolbar->addAction(m_propertiesAction);
    toolbar->addAction(m_createPatchAction);
    toolbar->addAction(m_logAction);
}

void MainWindow::setWorkingCopy(const QString &path)
{
    m_workingCopy = QDir(path).absolutePath();
    m_pathEdit->setText(m_workingCopy);
    m_fileModel->setRootPath(m_workingCopy);
    m_treeView->setRootIndex(m_fileModel->index(m_workingCopy));
    updateActionState();
    refreshStatus();
}

bool MainWindow::ensureWorkingCopy()
{
    if (!m_workingCopy.isEmpty()) {
        return true;
    }

    QMessageBox::information(this, QStringLiteral("qsvn"), QStringLiteral("Open or checkout a working copy first."));
    return false;
}

QString MainWindow::selectedPath() const
{
    const QList<QTableWidgetItem *> selectedItems = m_statusTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        const int row = selectedItems.first()->row();
        const QString relativePath = m_statusTable->item(row, 1)->text();
        return QDir(m_workingCopy).absoluteFilePath(relativePath);
    }

    const QModelIndex current = m_treeView->currentIndex();
    if (current.isValid()) {
        return m_fileModel->filePath(current);
    }

    return m_workingCopy;
}

QStringList MainWindow::selectedStatusPaths() const
{
    QStringList paths;
    const QModelIndexList rows = m_statusTable->selectionModel()->selectedRows();
    for (const QModelIndex &row : rows) {
        const auto *item = m_statusTable->item(row.row(), 1);
        if (item != nullptr) {
            paths.push_back(item->text());
        }
    }
    return paths;
}

QStringList MainWindow::selectedCommandPaths() const
{
    QStringList paths = selectedStatusPaths();
    if (paths.isEmpty()) {
        const QString path = selectedPath();
        if (!path.isEmpty()) {
            paths.push_back(path);
        }
    }
    return paths;
}

QString MainWindow::workingCopyRootForPath(const QString &path) const
{
    if (path.trimmed().isEmpty()) {
        return QString();
    }

    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    SvnResult result = m_svn.run({QStringLiteral("info"), QStringLiteral("--show-item"), QStringLiteral("wc-root"), absolutePath});
    if (result.ok()) {
        return result.standardOutput.trimmed();
    }

    result = m_svn.run({QStringLiteral("info"), absolutePath});
    if (!result.ok()) {
        return QString();
    }

    const QStringList lines = result.standardOutput.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith(QStringLiteral("Working Copy Root Path:"))) {
            return line.mid(QStringLiteral("Working Copy Root Path:").size()).trimmed();
        }
    }

    return QFileInfo(absolutePath).isDir() ? absolutePath : QFileInfo(absolutePath).absolutePath();
}

QString MainWindow::workingCopyUrl() const
{
    if (m_workingCopy.isEmpty()) {
        return QString();
    }

    SvnResult result = m_svn.run({QStringLiteral("info"), QStringLiteral("--show-item"), QStringLiteral("url")}, m_workingCopy);
    if (result.ok()) {
        return result.standardOutput.trimmed();
    }

    result = m_svn.run({QStringLiteral("info")}, m_workingCopy);
    if (!result.ok()) {
        return QString();
    }

    const QStringList lines = result.standardOutput.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith(QStringLiteral("URL:"))) {
            return line.mid(4).trimmed();
        }
    }

    return QString();
}

QString MainWindow::workingCopyRepositoryRootUrl() const
{
    if (m_workingCopy.isEmpty()) {
        return QString();
    }

    SvnResult result = m_svn.run({QStringLiteral("info"), QStringLiteral("--show-item"), QStringLiteral("repos-root-url")}, m_workingCopy);
    if (result.ok()) {
        return result.standardOutput.trimmed();
    }

    result = m_svn.run({QStringLiteral("info")}, m_workingCopy);
    if (!result.ok()) {
        return QString();
    }

    const QStringList lines = result.standardOutput.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith(QStringLiteral("Repository Root:"))) {
            return line.mid(QStringLiteral("Repository Root:").size()).trimmed();
        }
    }

    return QString();
}

void MainWindow::refreshStatus()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("status")}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Status failed"), result.combinedOutput());
        return;
    }

    m_statuses = m_svn.parseStatus(result.standardOutput);
    m_statusTable->setRowCount(m_statuses.size());
    for (int row = 0; row < m_statuses.size(); ++row) {
        m_statusTable->setItem(row, 0, new QTableWidgetItem(m_statuses.at(row).code));
        m_statusTable->setItem(row, 1, new QTableWidgetItem(m_statuses.at(row).path));
    }

    statusBar()->showMessage(QStringLiteral("%1 local change(s)").arg(m_statuses.size()));
}

void MainWindow::appendResult(const SvnResult &result)
{
    m_outputView->appendPlainText(QStringLiteral("> ") + result.commandLine);
    const QString output = result.combinedOutput().trimmed();
    if (!output.isEmpty()) {
        m_outputView->appendPlainText(output);
    }
    m_outputView->appendPlainText(QString());
}

void MainWindow::showTextDialog(const QString &title, const QString &text)
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle(title);
    dialog->resize(900, 650);

    auto *viewer = new QPlainTextEdit(dialog);
    viewer->setReadOnly(true);
    viewer->setPlainText(text);

    auto *closeButton = new QPushButton(QStringLiteral("Close"), dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    auto *layout = new QVBoxLayout(dialog);
    layout->addWidget(viewer, 1);
    layout->addWidget(closeButton, 0, Qt::AlignRight);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::runWorkingCopyCommand(const QStringList &arguments, bool refreshAfter)
{
    if (!ensureWorkingCopy()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run(arguments, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("SVN command failed"), result.combinedOutput());
        return;
    }

    if (refreshAfter) {
        refreshStatus();
    }
}

void MainWindow::runCheckout(const QString &url, const QString &target)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("checkout"), url, target});
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Checkout failed"), result.combinedOutput());
        return;
    }

    setWorkingCopy(target);
}

bool MainWindow::launchExternalDiff(const QString &path)
{
    if (m_settings.externalDiffCommand.isEmpty()) {
        return false;
    }

    const QFileInfo fileInfo(path);
    if (!fileInfo.exists() || fileInfo.isDir()) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("cat"), QStringLiteral("-r"), QStringLiteral("BASE"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        return false;
    }

    QString cacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheRoot.isEmpty()) {
        cacheRoot = QDir::tempPath() + QStringLiteral("/qsvn");
    }

    QDir cacheDir(cacheRoot);
    if (!cacheDir.mkpath(QStringLiteral("diff"))) {
        return false;
    }
    cacheDir.cd(QStringLiteral("diff"));

    const QString baseFileName = fileInfo.fileName() + QStringLiteral(".base.") + QString::number(QDateTime::currentMSecsSinceEpoch());
    const QString basePath = cacheDir.absoluteFilePath(baseFileName);

    QFile baseFile(basePath);
    if (!baseFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    baseFile.write(result.standardOutput.toLocal8Bit());
    baseFile.close();

    const ExternalToolCommand command = buildExternalToolCommand(m_settings.externalDiffCommand,
                                                                {
                                                                    {QStringLiteral("base"), basePath},
                                                                    {QStringLiteral("working"), path},
                                                                    {QStringLiteral("path"), path},
                                                                },
                                                                {basePath, path});
    if (!command.isValid()) {
        return false;
    }

    if (!QProcess::startDetached(command.program, command.arguments)) {
        QMessageBox::warning(this, QStringLiteral("External diff failed"), QStringLiteral("Could not start the configured external diff command."));
        return false;
    }

    m_outputView->appendPlainText(QStringLiteral("> external diff ") + m_settings.externalDiffCommand);
    return true;
}

bool MainWindow::launchExternalMerge(const QString &path)
{
    if (m_settings.externalMergeCommand.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Edit Conflict"), QStringLiteral("Configure an external merge command in Settings first."));
        return false;
    }

    const QFileInfo fileInfo(path);
    if (!fileInfo.exists() || fileInfo.isDir()) {
        return false;
    }

    const QString minePath = path + QStringLiteral(".mine");
    if (!QFileInfo::exists(minePath)) {
        QMessageBox::information(this, QStringLiteral("Edit Conflict"), QStringLiteral("No .mine conflict file was found for the selected path."));
        return false;
    }

    QDir directory(fileInfo.absolutePath());
    const QStringList revisionFiles = directory.entryList({fileInfo.fileName() + QStringLiteral(".r*")}, QDir::Files, QDir::Name);
    if (revisionFiles.size() < 2) {
        QMessageBox::information(this, QStringLiteral("Edit Conflict"), QStringLiteral("Could not find both base and incoming revision files."));
        return false;
    }

    QVector<QPair<int, QString>> revisions;
    for (const QString &fileName : revisionFiles) {
        bool ok = false;
        const int revision = fileName.mid(fileInfo.fileName().size() + 2).toInt(&ok);
        revisions.push_back({ok ? revision : 0, directory.absoluteFilePath(fileName)});
    }
    std::sort(revisions.begin(), revisions.end(), [](const auto &left, const auto &right) {
        return left.first < right.first;
    });

    const QString basePath = revisions.first().second;
    const QString theirsPath = revisions.last().second;
    const ExternalToolCommand command = buildExternalToolCommand(m_settings.externalMergeCommand,
                                                                {
                                                                    {QStringLiteral("base"), basePath},
                                                                    {QStringLiteral("mine"), minePath},
                                                                    {QStringLiteral("theirs"), theirsPath},
                                                                    {QStringLiteral("working"), path},
                                                                    {QStringLiteral("path"), path},
                                                                },
                                                                {basePath, minePath, theirsPath, path});
    if (!command.isValid()) {
        return false;
    }

    if (!QProcess::startDetached(command.program, command.arguments)) {
        QMessageBox::warning(this, QStringLiteral("External merge failed"), QStringLiteral("Could not start the configured external merge command."));
        return false;
    }

    m_outputView->appendPlainText(QStringLiteral("> external merge ") + m_settings.externalMergeCommand);
    return true;
}

void MainWindow::openWorkingCopy()
{
    const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Open working copy"));
    if (directory.isEmpty()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("info"), directory}, directory);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Open working copy"), QStringLiteral("The selected directory is not an SVN working copy."));
        return;
    }

    setWorkingCopy(directory);
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(m_settings, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_settings = dialog.settings();
    m_settings.save();
    m_svn.setOptions(m_settings.svn);
    statusBar()->showMessage(QStringLiteral("Settings saved."));
}

void MainWindow::checkout()
{
    CheckoutDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    runCheckout(dialog.repositoryUrl(), dialog.targetPath());
}

void MainWindow::exportProject()
{
    ExportDialog dialog(this);
    if (!m_workingCopy.isEmpty()) {
        dialog.setSourceUrl(workingCopyUrl());
    }
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QStringList arguments = {QStringLiteral("export")};
    if (dialog.force()) {
        arguments.append(QStringLiteral("--force"));
    }
    if (!dialog.revision().isEmpty()) {
        arguments.append({QStringLiteral("-r"), dialog.revision()});
    }
    arguments.append({dialog.sourceUrl(), dialog.targetPath()});

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run(arguments);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Export failed"), result.combinedOutput());
        return;
    }

    statusBar()->showMessage(QStringLiteral("Export completed."));
}

void MainWindow::importProject()
{
    ImportDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({
        QStringLiteral("import"),
        dialog.localPath(),
        dialog.targetUrl(),
        QStringLiteral("-m"),
        dialog.message(),
    });
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Import failed"), result.combinedOutput());
        return;
    }

    statusBar()->showMessage(QStringLiteral("Import completed."));
}

void MainWindow::browseRepository()
{
    RepositoryBrowserDialog browser(&m_svn, this);
    connect(&browser, &RepositoryBrowserDialog::commandFinished, this, &MainWindow::appendResult);
    if (browser.exec() != QDialog::Accepted) {
        return;
    }

    CheckoutDialog checkoutDialog(this);
    checkoutDialog.setRepositoryUrl(browser.checkoutUrl());
    if (checkoutDialog.exec() != QDialog::Accepted) {
        return;
    }

    runCheckout(checkoutDialog.repositoryUrl(), checkoutDialog.targetPath());
}

void MainWindow::branchOrTag()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    BranchTagDialog dialog(this);
    dialog.setSourceUrl(workingCopyUrl());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    runWorkingCopyCommand({
                              QStringLiteral("copy"),
                              dialog.sourceUrl(),
                              dialog.targetUrl(),
                              QStringLiteral("-m"),
                              dialog.message(),
                          },
                          false);
}

void MainWindow::switchWorkingCopy()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    SwitchDialog dialog(this);
    dialog.setTargetUrl(workingCopyUrl());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QStringList arguments = {QStringLiteral("switch")};
    if (!dialog.revision().isEmpty()) {
        arguments.append({QStringLiteral("-r"), dialog.revision()});
    }
    arguments.append(dialog.targetUrl());
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::relocateWorkingCopy()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    RelocateDialog dialog(this);
    dialog.setCurrentUrl(workingCopyUrl());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    runWorkingCopyCommand({QStringLiteral("relocate"), dialog.targetUrl(), m_workingCopy}, true);
}

void MainWindow::mergeIntoWorkingCopy()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    MergeDialog dialog(this);
    dialog.setSourceUrl(workingCopyUrl());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QStringList arguments = {QStringLiteral("merge")};
    if (dialog.dryRun()) {
        arguments.append(QStringLiteral("--dry-run"));
    }
    if (!dialog.revisionRange().isEmpty()) {
        arguments.append({QStringLiteral("-r"), dialog.revisionRange()});
    }
    arguments.append(dialog.sourceUrl());
    runWorkingCopyCommand(arguments, !dialog.dryRun());
}

void MainWindow::updateWorkingCopy()
{
    runWorkingCopyCommand({QStringLiteral("update")}, true);
}

void MainWindow::updateToRevision()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    bool ok = false;
    const QString revision = QInputDialog::getText(this,
                                                  QStringLiteral("Update to Revision"),
                                                  QStringLiteral("Revision:"),
                                                  QLineEdit::Normal,
                                                  QStringLiteral("HEAD"),
                                                  &ok)
                                 .trimmed();
    if (!ok || revision.isEmpty()) {
        return;
    }

    QStringList arguments = {QStringLiteral("update"), QStringLiteral("-r"), revision};
    arguments.append(selectedCommandPaths());
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::commitChanges()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    if (m_statuses.isEmpty()) {
        refreshStatus();
    }

    CommitDialog dialog(m_statuses, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QStringList arguments = {QStringLiteral("commit"), QStringLiteral("-m"), dialog.message()};
    arguments.append(dialog.selectedPaths());
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::addSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    if (paths.isEmpty()) {
        return;
    }

    QStringList arguments = {QStringLiteral("add")};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::ignoreSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString path = selectedPath();
    if (path.isEmpty()) {
        return;
    }

    ignorePath(path);
}

void MainWindow::copySelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString path = selectedPath();
    if (path.isEmpty()) {
        return;
    }

    copyPath(path);
}

void MainWindow::deleteSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    if (paths.isEmpty()) {
        return;
    }

    const QString message = QStringLiteral("Delete %1 selected path(s) with SVN?").arg(paths.size());
    if (QMessageBox::question(this, QStringLiteral("Delete"), message) != QMessageBox::Yes) {
        return;
    }

    QStringList arguments = {QStringLiteral("delete")};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::renameSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString path = selectedPath();
    if (path.isEmpty()) {
        return;
    }

    renamePath(path);
}

void MainWindow::revertSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    const QString message = QStringLiteral("Revert %1 selected path(s)?").arg(paths.size());
    if (QMessageBox::question(this, QStringLiteral("Revert"), message) != QMessageBox::Yes) {
        return;
    }

    QStringList arguments = {QStringLiteral("revert")};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::setChangelist()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    if (paths.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString name = QInputDialog::getText(this,
                                               QStringLiteral("Set Changelist"),
                                               QStringLiteral("Changelist name:"),
                                               QLineEdit::Normal,
                                               QString(),
                                               &ok)
                             .trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }

    QStringList arguments = {QStringLiteral("changelist"), name};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::removeChangelist()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    if (paths.isEmpty()) {
        return;
    }

    QStringList arguments = {QStringLiteral("changelist"), QStringLiteral("--remove")};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::cleanupWorkingCopy()
{
    runWorkingCopyCommand({QStringLiteral("cleanup")}, true);
}

void MainWindow::resolveSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    if (paths.isEmpty()) {
        return;
    }

    const QStringList choices = {
        QStringLiteral("working"),
        QStringLiteral("mine-conflict"),
        QStringLiteral("theirs-conflict"),
        QStringLiteral("base"),
    };

    bool ok = false;
    const QString choice = QInputDialog::getItem(this,
                                                QStringLiteral("Resolve"),
                                                QStringLiteral("Accept:"),
                                                choices,
                                                0,
                                                false,
                                                &ok);
    if (!ok || choice.isEmpty()) {
        return;
    }

    QStringList arguments = {QStringLiteral("resolve"), QStringLiteral("--accept"), choice};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::showConflicts()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString path = selectedPath();
    if (!path.isEmpty()) {
        showConflictsForPath(path);
    }
}

void MainWindow::editConflict()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString path = selectedPath();
    if (path.isEmpty()) {
        return;
    }

    launchExternalMerge(path);
}

void MainWindow::lockSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    if (paths.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString message = QInputDialog::getMultiLineText(this,
                                                          QStringLiteral("Lock"),
                                                          QStringLiteral("Lock message:"),
                                                          QString(),
                                                          &ok);
    if (!ok) {
        return;
    }

    QStringList arguments = {QStringLiteral("lock"), QStringLiteral("-m"), message};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::unlockSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QStringList paths = selectedCommandPaths();
    if (paths.isEmpty()) {
        return;
    }

    QStringList arguments = {QStringLiteral("unlock")};
    arguments.append(paths);
    runWorkingCopyCommand(arguments, true);
}

void MainWindow::diffSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    QStringList arguments = {QStringLiteral("diff"), QStringLiteral("--internal-diff")};
    const QString path = selectedPath();
    if (!path.isEmpty()) {
        if (launchExternalDiff(path)) {
            return;
        }
        arguments.push_back(path);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run(arguments, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Diff failed"), result.combinedOutput());
        return;
    }

    showTextDialog(QStringLiteral("Diff"), result.standardOutput);
}

void MainWindow::createPatch()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    QStringList arguments = {QStringLiteral("diff"), QStringLiteral("--internal-diff")};
    const QStringList statusPaths = selectedStatusPaths();
    if (!statusPaths.isEmpty()) {
        arguments.append(statusPaths);
    } else {
        const QString path = selectedPath();
        if (!path.isEmpty() && path != m_workingCopy) {
            arguments.append(path);
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run(arguments, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Create Patch failed"), result.combinedOutput());
        return;
    }
    if (result.standardOutput.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Create Patch"), QStringLiteral("There are no local diff lines to save."));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(this,
                                                         QStringLiteral("Save Patch"),
                                                         QDir(m_workingCopy).absoluteFilePath(QStringLiteral("changes.patch")),
                                                         QStringLiteral("Patch files (*.patch *.diff);;All files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile patchFile(fileName);
    if (!patchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("Create Patch failed"), QStringLiteral("Could not write the patch file."));
        return;
    }
    patchFile.write(result.standardOutput.toLocal8Bit());
    patchFile.close();

    statusBar()->showMessage(QStringLiteral("Patch saved."));
}

void MainWindow::applyPatch()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString fileName = QFileDialog::getOpenFileName(this,
                                                         QStringLiteral("Apply Patch"),
                                                         m_workingCopy,
                                                         QStringLiteral("Patch files (*.patch *.diff);;All files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    runWorkingCopyCommand({QStringLiteral("patch"), fileName}, true);
}

void MainWindow::openProperties()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString path = selectedPath();
    if (path.isEmpty()) {
        return;
    }

    auto *dialog = new PropertiesDialog(&m_svn, m_workingCopy, path, this);
    connect(dialog, &PropertiesDialog::commandFinished, this, &MainWindow::appendResult);
    connect(dialog, &QDialog::finished, this, [this]() {
        refreshStatus();
    });
    dialog->refresh();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::logSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    QStringList arguments = {QStringLiteral("log"), QStringLiteral("--xml"), QStringLiteral("-v"), QStringLiteral("-l"), QStringLiteral("100")};
    const QString path = selectedPath();
    if (!path.isEmpty()) {
        arguments.push_back(path);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run(arguments, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Log failed"), result.combinedOutput());
        return;
    }

    const QVector<SvnLogEntry> entries = m_svn.parseLogXml(result.standardOutput);
    if (entries.isEmpty() && !result.standardOutput.trimmed().isEmpty()) {
        showTextDialog(QStringLiteral("Log"), result.standardOutput);
        return;
    }

    auto *dialog = new LogDialog(entries, this);
    connect(dialog, &LogDialog::revisionDiffRequested, this, [this, path](const QString &revision) {
        QStringList diffArguments = {QStringLiteral("diff"), QStringLiteral("--internal-diff"), QStringLiteral("-c"), revision};
        if (!path.isEmpty()) {
            diffArguments.push_back(path);
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        const SvnResult diffResult = m_svn.run(diffArguments, m_workingCopy);
        QApplication::restoreOverrideCursor();
        appendResult(diffResult);

        if (!diffResult.ok()) {
            QMessageBox::warning(this, QStringLiteral("Revision diff failed"), diffResult.combinedOutput());
            return;
        }

        showTextDialog(QStringLiteral("Revision ") + revision + QStringLiteral(" Diff"), diffResult.standardOutput);
    });
    connect(dialog, &LogDialog::revisionBlameRequested, this, [this](const QString &revision, const QString &repositoryPath) {
        QString blameTarget = repositoryPath;
        if (repositoryPath.startsWith('/')) {
            const QString rootUrl = workingCopyRepositoryRootUrl();
            if (!rootUrl.isEmpty()) {
                blameTarget = rootUrl + repositoryPath;
            }
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        const SvnResult blameResult = m_svn.run({QStringLiteral("blame"), QStringLiteral("-r"), revision, blameTarget}, m_workingCopy);
        QApplication::restoreOverrideCursor();
        appendResult(blameResult);

        if (!blameResult.ok()) {
            QMessageBox::warning(this, QStringLiteral("Blame failed"), blameResult.combinedOutput());
            return;
        }

        showTextDialog(QStringLiteral("Blame r") + revision, blameResult.standardOutput);
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::blameSelected()
{
    if (!ensureWorkingCopy()) {
        return;
    }

    const QString path = selectedPath();
    if (path.isEmpty()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svn.run({QStringLiteral("blame"), path}, m_workingCopy);
    QApplication::restoreOverrideCursor();
    appendResult(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Blame failed"), result.combinedOutput());
        return;
    }

    showTextDialog(QStringLiteral("Blame"), result.standardOutput);
}

void MainWindow::updateActionState()
{
    const bool hasWorkingCopy = !m_workingCopy.isEmpty();
    m_branchTagAction->setEnabled(hasWorkingCopy);
    m_switchAction->setEnabled(hasWorkingCopy);
    m_relocateAction->setEnabled(hasWorkingCopy);
    m_mergeAction->setEnabled(hasWorkingCopy);
    m_refreshAction->setEnabled(hasWorkingCopy);
    m_updateAction->setEnabled(hasWorkingCopy);
    m_updateRevisionAction->setEnabled(hasWorkingCopy);
    m_commitAction->setEnabled(hasWorkingCopy);
    m_addAction->setEnabled(hasWorkingCopy);
    m_ignoreAction->setEnabled(hasWorkingCopy);
    m_copyAction->setEnabled(hasWorkingCopy);
    m_deleteAction->setEnabled(hasWorkingCopy);
    m_renameAction->setEnabled(hasWorkingCopy);
    m_revertAction->setEnabled(hasWorkingCopy);
    m_setChangelistAction->setEnabled(hasWorkingCopy);
    m_removeChangelistAction->setEnabled(hasWorkingCopy);
    m_cleanupAction->setEnabled(hasWorkingCopy);
    m_resolveAction->setEnabled(hasWorkingCopy);
    m_conflictsAction->setEnabled(hasWorkingCopy);
    m_editConflictAction->setEnabled(hasWorkingCopy);
    m_lockAction->setEnabled(hasWorkingCopy);
    m_unlockAction->setEnabled(hasWorkingCopy);
    m_diffAction->setEnabled(hasWorkingCopy);
    m_createPatchAction->setEnabled(hasWorkingCopy);
    m_applyPatchAction->setEnabled(hasWorkingCopy);
    m_propertiesAction->setEnabled(hasWorkingCopy);
    m_logAction->setEnabled(hasWorkingCopy);
    m_blameAction->setEnabled(hasWorkingCopy);
}
