#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;

class MergeDialog : public QDialog {
    Q_OBJECT

public:
    explicit MergeDialog(QWidget *parent = nullptr);

    void setSourceUrl(const QString &url);
    QString sourceUrl() const;
    QString revisionRange() const;
    bool dryRun() const;

private:
    void validateAndAccept();

    QLineEdit *m_sourceEdit = nullptr;
    QLineEdit *m_revisionRangeEdit = nullptr;
    QCheckBox *m_dryRunCheck = nullptr;
};
