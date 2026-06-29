#pragma once

#include "ApplicationSettings.h"
#include "SvnClient.h"

#include <QFileSystemModel>
#include <QMainWindow>
#include <QString>
#include <QVector>

class QAction;
class QLineEdit;
class QPlainTextEdit;
class QSplitter;
class QTableWidget;
class QTreeView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool openPath(const QString &path);
    bool updatePath(const QString &path);
    bool commitPath(const QString &path);
    bool addPath(const QString &path);
    bool ignorePath(const QString &path);
    bool copyPath(const QString &path);
    bool deletePath(const QString &path);
    bool renamePath(const QString &path);
    bool revertPath(const QString &path);
    bool cleanupPath(const QString &path);
    bool showDiffForPath(const QString &path);
    bool showLogForPath(const QString &path);
    bool showBlameForPath(const QString &path);
    bool showPropertiesForPath(const QString &path);
    bool showConflictsForPath(const QString &path);
    void showRepositoryBrowser();

private:
    void createActions();
    void createCentralWidget();
    void createMenus();
    void createToolBar();
    void setWorkingCopy(const QString &path);
    bool ensureWorkingCopy();
    QString selectedPath() const;
    QStringList selectedStatusPaths() const;
    QStringList selectedCommandPaths() const;
    QString workingCopyRootForPath(const QString &path) const;
    QString workingCopyUrl() const;
    QString workingCopyRepositoryRootUrl() const;
    void refreshStatus();
    void appendResult(const SvnResult &result);
    void showTextDialog(const QString &title, const QString &text);
    void runWorkingCopyCommand(const QStringList &arguments, bool refreshAfter);
    void runCheckout(const QString &url, const QString &target);
    bool launchExternalDiff(const QString &path);
    bool launchExternalMerge(const QString &path);

    void openWorkingCopy();
    void openSettings();
    void checkout();
    void exportProject();
    void importProject();
    void browseRepository();
    void branchOrTag();
    void switchWorkingCopy();
    void relocateWorkingCopy();
    void mergeIntoWorkingCopy();
    void updateWorkingCopy();
    void updateToRevision();
    void commitChanges();
    void addSelected();
    void ignoreSelected();
    void copySelected();
    void deleteSelected();
    void renameSelected();
    void revertSelected();
    void setChangelist();
    void removeChangelist();
    void cleanupWorkingCopy();
    void resolveSelected();
    void showConflicts();
    void editConflict();
    void lockSelected();
    void unlockSelected();
    void diffSelected();
    void createPatch();
    void applyPatch();
    void openProperties();
    void logSelected();
    void blameSelected();
    void updateActionState();

    SvnClient m_svn;
    ApplicationSettings m_settings;
    QString m_workingCopy;
    QVector<SvnStatus> m_statuses;

    QFileSystemModel *m_fileModel = nullptr;
    QTreeView *m_treeView = nullptr;
    QTableWidget *m_statusTable = nullptr;
    QPlainTextEdit *m_outputView = nullptr;
    QLineEdit *m_pathEdit = nullptr;

    QAction *m_openAction = nullptr;
    QAction *m_settingsAction = nullptr;
    QAction *m_checkoutAction = nullptr;
    QAction *m_exportAction = nullptr;
    QAction *m_importAction = nullptr;
    QAction *m_repoBrowserAction = nullptr;
    QAction *m_branchTagAction = nullptr;
    QAction *m_switchAction = nullptr;
    QAction *m_relocateAction = nullptr;
    QAction *m_mergeAction = nullptr;
    QAction *m_refreshAction = nullptr;
    QAction *m_updateAction = nullptr;
    QAction *m_updateRevisionAction = nullptr;
    QAction *m_commitAction = nullptr;
    QAction *m_addAction = nullptr;
    QAction *m_ignoreAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_deleteAction = nullptr;
    QAction *m_renameAction = nullptr;
    QAction *m_revertAction = nullptr;
    QAction *m_setChangelistAction = nullptr;
    QAction *m_removeChangelistAction = nullptr;
    QAction *m_cleanupAction = nullptr;
    QAction *m_resolveAction = nullptr;
    QAction *m_conflictsAction = nullptr;
    QAction *m_editConflictAction = nullptr;
    QAction *m_lockAction = nullptr;
    QAction *m_unlockAction = nullptr;
    QAction *m_diffAction = nullptr;
    QAction *m_createPatchAction = nullptr;
    QAction *m_applyPatchAction = nullptr;
    QAction *m_propertiesAction = nullptr;
    QAction *m_logAction = nullptr;
    QAction *m_blameAction = nullptr;
};
