#include "CheckoutDialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CheckoutDialog::CheckoutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowTitle(QStringLiteral("Checkout"));
    setMinimumWidth(560);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText(QStringLiteral("https://example.com/svn/project/trunk"));

    m_targetEdit = new QLineEdit(this);
    m_targetEdit->setPlaceholderText(QStringLiteral("/home/user/project"));

    auto *browseButton = new QPushButton(QStringLiteral("Browse..."), this);
    connect(browseButton, &QPushButton::clicked, this, &CheckoutDialog::browseTarget);

    auto *targetLayout = new QHBoxLayout;
    targetLayout->addWidget(m_targetEdit, 1);
    targetLayout->addWidget(browseButton);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("Repository URL:"), m_urlEdit);
    form->addRow(QStringLiteral("Checkout directory:"), targetLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &CheckoutDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString CheckoutDialog::repositoryUrl() const
{
    return m_urlEdit->text().trimmed();
}

QString CheckoutDialog::targetPath() const
{
    return m_targetEdit->text().trimmed();
}

void CheckoutDialog::setRepositoryUrl(const QString &url)
{
    m_urlEdit->setText(url);
}

void CheckoutDialog::setTargetPath(const QString &path)
{
    m_targetEdit->setText(path);
}

void CheckoutDialog::browseTarget()
{
    const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Checkout directory"));
    if (!directory.isEmpty()) {
        m_targetEdit->setText(directory);
    }
}

void CheckoutDialog::validateAndAccept()
{
    if (repositoryUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Checkout"), QStringLiteral("Repository URL is required."));
        return;
    }
    if (targetPath().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Checkout"), QStringLiteral("Checkout directory is required."));
        return;
    }
    accept();
}
