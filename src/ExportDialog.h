#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);

    void setSourceUrl(const QString &url);
    QString sourceUrl() const;
    QString targetPath() const;
    QString revision() const;
    bool force() const;

private:
    void browseTarget();
    void validateAndAccept();

    QLineEdit *m_sourceEdit = nullptr;
    QLineEdit *m_targetEdit = nullptr;
    QLineEdit *m_revisionEdit = nullptr;
    QCheckBox *m_forceCheck = nullptr;
};
