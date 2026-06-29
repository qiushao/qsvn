#include "SettingsDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(const ApplicationSettings &settings, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Settings"));
    setMinimumWidth(640);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setText(settings.svn.username);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setText(settings.svn.password);

    m_useAuthCacheCheck = new QCheckBox(QStringLiteral("Use SVN authentication cache"), this);
    m_useAuthCacheCheck->setChecked(settings.svn.useAuthCache);

    m_trustCertificateCheck = new QCheckBox(QStringLiteral("Trust server certificate for non-interactive commands"), this);
    m_trustCertificateCheck->setChecked(settings.svn.trustServerCertificate);

    m_diffCommandEdit = new QLineEdit(this);
    m_diffCommandEdit->setText(settings.externalDiffCommand);
    m_diffCommandEdit->setPlaceholderText(QStringLiteral("meld {base} {working}"));

    auto *browseDiffButton = new QPushButton(QStringLiteral("Browse..."), this);
    connect(browseDiffButton, &QPushButton::clicked, this, &SettingsDialog::browseDiffCommand);

    auto *diffLayout = new QHBoxLayout;
    diffLayout->addWidget(m_diffCommandEdit, 1);
    diffLayout->addWidget(browseDiffButton);

    m_mergeCommandEdit = new QLineEdit(this);
    m_mergeCommandEdit->setText(settings.externalMergeCommand);
    m_mergeCommandEdit->setPlaceholderText(QStringLiteral("kdiff3 {base} {mine} {theirs} -o {working}"));

    auto *browseMergeButton = new QPushButton(QStringLiteral("Browse..."), this);
    connect(browseMergeButton, &QPushButton::clicked, this, &SettingsDialog::browseMergeCommand);

    auto *mergeLayout = new QHBoxLayout;
    mergeLayout->addWidget(m_mergeCommandEdit, 1);
    mergeLayout->addWidget(browseMergeButton);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("Username:"), m_usernameEdit);
    form->addRow(QStringLiteral("Password:"), m_passwordEdit);
    form->addRow(QString(), m_useAuthCacheCheck);
    form->addRow(QString(), m_trustCertificateCheck);
    form->addRow(QStringLiteral("External diff:"), diffLayout);
    form->addRow(QStringLiteral("External merge:"), mergeLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

ApplicationSettings SettingsDialog::settings() const
{
    ApplicationSettings values;
    values.svn.username = m_usernameEdit->text().trimmed();
    values.svn.password = m_passwordEdit->text();
    values.svn.useAuthCache = m_useAuthCacheCheck->isChecked();
    values.svn.trustServerCertificate = m_trustCertificateCheck->isChecked();
    values.externalDiffCommand = m_diffCommandEdit->text().trimmed();
    values.externalMergeCommand = m_mergeCommandEdit->text().trimmed();
    return values;
}

void SettingsDialog::browseDiffCommand()
{
    browseCommand(m_diffCommandEdit);
}

void SettingsDialog::browseMergeCommand()
{
    browseCommand(m_mergeCommandEdit);
}

void SettingsDialog::browseCommand(QLineEdit *targetEdit)
{
    const QString file = QFileDialog::getOpenFileName(this, QStringLiteral("External diff command"));
    if (!file.isEmpty()) {
        targetEdit->setText(file);
    }
}
