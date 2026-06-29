#pragma once

#include <QDialog>

class QLineEdit;

class SwitchDialog : public QDialog {
    Q_OBJECT

public:
    explicit SwitchDialog(QWidget *parent = nullptr);

    void setTargetUrl(const QString &url);
    QString targetUrl() const;
    QString revision() const;

private:
    void validateAndAccept();

    QLineEdit *m_urlEdit = nullptr;
    QLineEdit *m_revisionEdit = nullptr;
};
