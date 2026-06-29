#include "RelocateDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

RelocateDialog::RelocateDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowTitle(QStringLiteral("Relocate"));
    setMinimumWidth(680);

    m_currentUrlEdit = new QLineEdit(this);
    m_currentUrlEdit->setReadOnly(true);

    m_targetUrlEdit = new QLineEdit(this);
    m_targetUrlEdit->setPlaceholderText(QStringLiteral("New repository root or working copy URL"));

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("Current URL:"), m_currentUrlEdit);
    form->addRow(QStringLiteral("New URL:"), m_targetUrlEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &RelocateDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void RelocateDialog::setCurrentUrl(const QString &url)
{
    m_currentUrlEdit->setText(url);
}

QString RelocateDialog::currentUrl() const
{
    return m_currentUrlEdit->text().trimmed();
}

QString RelocateDialog::targetUrl() const
{
    return m_targetUrlEdit->text().trimmed();
}

void RelocateDialog::validateAndAccept()
{
    if (targetUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Relocate"), QStringLiteral("New URL is required."));
        return;
    }
    if (!currentUrl().isEmpty() && currentUrl() == targetUrl()) {
        QMessageBox::warning(this, QStringLiteral("Relocate"), QStringLiteral("New URL must differ from the current URL."));
        return;
    }

    accept();
}
