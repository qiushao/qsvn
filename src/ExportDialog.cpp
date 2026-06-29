#include "ExportDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowTitle(QStringLiteral("Export"));
    setMinimumWidth(640);

    m_sourceEdit = new QLineEdit(this);
    m_sourceEdit->setPlaceholderText(QStringLiteral("Repository URL or working copy path"));

    m_targetEdit = new QLineEdit(this);
    m_targetEdit->setPlaceholderText(QStringLiteral("/home/user/exported-project"));

    auto *browseButton = new QPushButton(QStringLiteral("Browse..."), this);
    connect(browseButton, &QPushButton::clicked, this, &ExportDialog::browseTarget);

    auto *targetLayout = new QHBoxLayout;
    targetLayout->addWidget(m_targetEdit, 1);
    targetLayout->addWidget(browseButton);

    m_revisionEdit = new QLineEdit(this);
    m_revisionEdit->setPlaceholderText(QStringLiteral("HEAD"));

    m_forceCheck = new QCheckBox(QStringLiteral("Overwrite existing files"), this);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("From:"), m_sourceEdit);
    form->addRow(QStringLiteral("To directory:"), targetLayout);
    form->addRow(QStringLiteral("Revision:"), m_revisionEdit);
    form->addRow(QString(), m_forceCheck);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &ExportDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void ExportDialog::setSourceUrl(const QString &url)
{
    m_sourceEdit->setText(url);
}

QString ExportDialog::sourceUrl() const
{
    return m_sourceEdit->text().trimmed();
}

QString ExportDialog::targetPath() const
{
    return m_targetEdit->text().trimmed();
}

QString ExportDialog::revision() const
{
    return m_revisionEdit->text().trimmed();
}

bool ExportDialog::force() const
{
    return m_forceCheck->isChecked();
}

void ExportDialog::browseTarget()
{
    const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Export target directory"));
    if (!directory.isEmpty()) {
        m_targetEdit->setText(directory);
    }
}

void ExportDialog::validateAndAccept()
{
    if (sourceUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Export"), QStringLiteral("Source URL or path is required."));
        return;
    }
    if (targetPath().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Export"), QStringLiteral("Target directory is required."));
        return;
    }

    accept();
}
