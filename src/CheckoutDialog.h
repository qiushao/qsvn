#pragma once

#include <QDialog>

class QLineEdit;

class CheckoutDialog : public QDialog {
    Q_OBJECT

public:
    explicit CheckoutDialog(QWidget *parent = nullptr);

    QString repositoryUrl() const;
    QString targetPath() const;
    void setRepositoryUrl(const QString &url);
    void setTargetPath(const QString &path);

private:
    void browseTarget();
    void validateAndAccept();

    QLineEdit *m_urlEdit = nullptr;
    QLineEdit *m_targetEdit = nullptr;
};
