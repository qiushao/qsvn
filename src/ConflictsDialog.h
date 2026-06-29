#pragma once

#include "SvnClient.h"

#include <QDialog>
#include <QVector>

class QTableWidget;

class ConflictsDialog : public QDialog {
    Q_OBJECT

public:
    ConflictsDialog(const QVector<SvnStatus> &conflicts, const QString &workingCopy, QWidget *parent = nullptr);

signals:
    void editConflictRequested(const QString &path);
    void resolveRequested(const QString &path, const QString &acceptChoice);

private:
    void editSelected();
    void resolveSelected();
    QString selectedPath() const;

    QVector<SvnStatus> m_conflicts;
    QString m_workingCopy;
    QTableWidget *m_table = nullptr;
};
