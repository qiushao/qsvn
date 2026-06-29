#pragma once

#include "SvnClient.h"

#include <QDialog>
#include <QString>
#include <QVector>

class QPlainTextEdit;
class QTableWidget;

class PropertiesDialog : public QDialog {
    Q_OBJECT

public:
    PropertiesDialog(SvnClient *svnClient, const QString &workingDirectory, const QString &targetPath, QWidget *parent = nullptr);
    void refresh();

signals:
    void commandFinished(const SvnResult &result);

private:
    void loadProperties();
    void setProperty();
    void deleteProperty();
    void showSelectedProperty(int row);
    QString selectedPropertyName() const;
    QString selectedPropertyValue() const;

    SvnClient *m_svnClient = nullptr;
    QString m_workingDirectory;
    QString m_targetPath;
    QVector<SvnProperty> m_properties;

    QTableWidget *m_table = nullptr;
    QPlainTextEdit *m_valueView = nullptr;
};
