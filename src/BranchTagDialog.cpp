#include "BranchTagDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>

BranchTagDialog::BranchTagDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowTitle(QStringLiteral("Branch / Tag"));
    resize(680, 360);

    m_sourceEdit = new QLineEdit(this);
    m_sourceEdit->setPlaceholderText(QStringLiteral("Repository URL to copy from"));

    m_targetEdit = new QLineEdit(this);
    m_targetEdit->setPlaceholderText(QStringLiteral("Repository URL for new branch or tag"));

    m_messageEdit = new QTextEdit(this);
    m_messageEdit->setPlaceholderText(QStringLiteral("Log message"));
    m_messageEdit->setMinimumHeight(100);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("From URL:"), m_sourceEdit);
    form->addRow(QStringLiteral("To URL:"), m_targetEdit);
    form->addRow(QStringLiteral("Message:"), m_messageEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &BranchTagDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void BranchTagDialog::setSourceUrl(const QString &url)
{
    m_sourceEdit->setText(url);
}

QString BranchTagDialog::sourceUrl() const
{
    return m_sourceEdit->text().trimmed();
}

QString BranchTagDialog::targetUrl() const
{
    return m_targetEdit->text().trimmed();
}

QString BranchTagDialog::message() const
{
    return m_messageEdit->toPlainText().trimmed();
}

void BranchTagDialog::validateAndAccept()
{
    if (sourceUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Branch / Tag"), QStringLiteral("Source URL is required."));
        return;
    }
    if (targetUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Branch / Tag"), QStringLiteral("Target URL is required."));
        return;
    }
    if (message().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Branch / Tag"), QStringLiteral("Log message is required."));
        return;
    }

    accept();
}
