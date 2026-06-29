#pragma once

#include "ApplicationSettings.h"

#include <QDialog>

class QCheckBox;
class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const ApplicationSettings &settings, QWidget *parent = nullptr);

    ApplicationSettings settings() const;

private:
    void browseDiffCommand();
    void browseMergeCommand();
    void browseCommand(QLineEdit *targetEdit);

    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QCheckBox *m_useAuthCacheCheck = nullptr;
    QCheckBox *m_trustCertificateCheck = nullptr;
    QLineEdit *m_diffCommandEdit = nullptr;
    QLineEdit *m_mergeCommandEdit = nullptr;
};
