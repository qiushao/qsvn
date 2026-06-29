#pragma once

#include "SvnClient.h"

#include <QDialog>
#include <QString>

class QLineEdit;
class QPushButton;
class QTableWidget;

class RepositoryBrowserDialog : public QDialog {
    Q_OBJECT

public:
    explicit RepositoryBrowserDialog(const SvnClient *svnClient, QWidget *parent = nullptr);

    QString checkoutUrl() const;

signals:
    void commandFinished(const SvnResult &result);

private:
    void listUrl();
    void goUp();
    void openSelected();
    void createFolder();
    void copySelected();
    void deleteSelected();
    void renameSelected();
    void acceptCheckout();
    QString selectedUrl() const;
    QString selectedName() const;
    QString currentOrSelectedUrl() const;
    QString appendPath(const QString &url, const QString &entry) const;
    QString parentUrl(const QString &url) const;

    const SvnClient *m_svnClient = nullptr;
    QLineEdit *m_urlEdit = nullptr;
    QTableWidget *m_table = nullptr;
    QPushButton *m_checkoutButton = nullptr;
    QString m_checkoutUrl;
};
