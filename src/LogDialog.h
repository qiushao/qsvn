#pragma once

#include "SvnClient.h"

#include <QDialog>
#include <QVector>

class QLineEdit;
class QPlainTextEdit;
class QTableWidget;

class LogDialog : public QDialog {
    Q_OBJECT

public:
    explicit LogDialog(const QVector<SvnLogEntry> &entries, QWidget *parent = nullptr);

signals:
    void revisionDiffRequested(const QString &revision);
    void changedPathDiffRequested(const QString &revision, const QString &repositoryPath, const QString &action);
    void revisionBlameRequested(const QString &revision, const QString &repositoryPath);

private:
    void showEntry(int row);
    void applyFilter();
    void requestRevisionDiff();
    void requestChangedPathDiff();
    void requestRevisionBlame();
    int currentEntryRow() const;
    QString currentChangedPath() const;
    QString changedPathActionFromChangedPath(const QString &changedPath) const;
    QString repositoryPathFromChangedPath(const QString &changedPath) const;

    QVector<SvnLogEntry> m_entries;
    QLineEdit *m_filterEdit = nullptr;
    QTableWidget *m_entryTable = nullptr;
    QPlainTextEdit *m_messageView = nullptr;
    QTableWidget *m_pathTable = nullptr;
};
