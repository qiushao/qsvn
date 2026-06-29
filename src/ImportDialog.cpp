#include "ImportDialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

ImportDialog::ImportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Import"));
    resize(680, 360);

    m_localPathEdit = new QLineEdit(this);
    m_localPathEdit->setPlaceholderText(QStringLiteral("/home/user/project"));

    auto *browseButton = new QPushButton(QStringLiteral("Browse..."), this);
    connect(browseButton, &QPushButton::clicked, this, &ImportDialog::browseLocalPath);

    auto *localLayout = new QHBoxLayout;
    localLayout->addWidget(m_localPathEdit, 1);
    localLayout->addWidget(browseButton);

    m_targetUrlEdit = new QLineEdit(this);
    m_targetUrlEdit->setPlaceholderText(QStringLiteral("https://example.com/svn/project/trunk"));

    m_messageEdit = new QTextEdit(this);
    m_messageEdit->setPlaceholderText(QStringLiteral("Import message"));
    m_messageEdit->setMinimumHeight(100);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("Local path:"), localLayout);
    form->addRow(QStringLiteral("Repository URL:"), m_targetUrlEdit);
    form->addRow(QStringLiteral("Message:"), m_messageEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &ImportDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString ImportDialog::localPath() const
{
    return m_localPathEdit->text().trimmed();
}

QString ImportDialog::targetUrl() const
{
    return m_targetUrlEdit->text().trimmed();
}

QString ImportDialog::message() const
{
    return m_messageEdit->toPlainText().trimmed();
}

void ImportDialog::browseLocalPath()
{
    const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Import local path"));
    if (!directory.isEmpty()) {
        m_localPathEdit->setText(directory);
    }
}

void ImportDialog::validateAndAccept()
{
    if (localPath().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Import"), QStringLiteral("Local path is required."));
        return;
    }
    if (targetUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Import"), QStringLiteral("Repository URL is required."));
        return;
    }
    if (message().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Import"), QStringLiteral("Import message is required."));
        return;
    }

    accept();
}
