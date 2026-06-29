#pragma once

#include <QDialog>

class QLineEdit;
class QTextEdit;

class BranchTagDialog : public QDialog {
    Q_OBJECT

public:
    explicit BranchTagDialog(QWidget *parent = nullptr);

    void setSourceUrl(const QString &url);
    QString sourceUrl() const;
    QString targetUrl() const;
    QString message() const;

private:
    void validateAndAccept();

    QLineEdit *m_sourceEdit = nullptr;
    QLineEdit *m_targetEdit = nullptr;
    QTextEdit *m_messageEdit = nullptr;
};
