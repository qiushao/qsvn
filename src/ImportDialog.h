#pragma once

#include <QDialog>

class QLineEdit;
class QTextEdit;

class ImportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = nullptr);

    void setLocalPath(const QString &path);
    QString localPath() const;
    QString targetUrl() const;
    QString message() const;

private:
    void browseLocalPath();
    void validateAndAccept();

    QLineEdit *m_localPathEdit = nullptr;
    QLineEdit *m_targetUrlEdit = nullptr;
    QTextEdit *m_messageEdit = nullptr;
};
