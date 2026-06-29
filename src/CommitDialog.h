#pragma once

#include "SvnClient.h"

#include <QDialog>
#include <QStringList>

class QListWidget;
class QTextEdit;

class CommitDialog : public QDialog {
    Q_OBJECT

public:
    explicit CommitDialog(const QVector<SvnStatus> &statuses, QWidget *parent = nullptr);

    QString message() const;
    QStringList selectedPaths() const;

private:
    void validateAndAccept();

    QTextEdit *m_messageEdit = nullptr;
    QListWidget *m_pathList = nullptr;
};
