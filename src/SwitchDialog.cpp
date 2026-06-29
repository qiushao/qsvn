#include "SwitchDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

SwitchDialog::SwitchDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Switch"));
    setMinimumWidth(620);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText(QStringLiteral("Repository URL to switch to"));

    m_revisionEdit = new QLineEdit(this);
    m_revisionEdit->setPlaceholderText(QStringLiteral("HEAD"));

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("To URL:"), m_urlEdit);
    form->addRow(QStringLiteral("Revision:"), m_revisionEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SwitchDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void SwitchDialog::setTargetUrl(const QString &url)
{
    m_urlEdit->setText(url);
}

QString SwitchDialog::targetUrl() const
{
    return m_urlEdit->text().trimmed();
}

QString SwitchDialog::revision() const
{
    return m_revisionEdit->text().trimmed();
}

void SwitchDialog::validateAndAccept()
{
    if (targetUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Switch"), QStringLiteral("Target URL is required."));
        return;
    }

    accept();
}
