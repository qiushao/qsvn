#pragma once

#include <QDialog>

class QLineEdit;

class RelocateDialog : public QDialog {
    Q_OBJECT

public:
    explicit RelocateDialog(QWidget *parent = nullptr);

    void setCurrentUrl(const QString &url);
    QString currentUrl() const;
    QString targetUrl() const;

private:
    void validateAndAccept();

    QLineEdit *m_currentUrlEdit = nullptr;
    QLineEdit *m_targetUrlEdit = nullptr;
};
