#include "MergeDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

MergeDialog::MergeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Merge"));
    setMinimumWidth(620);

    m_sourceEdit = new QLineEdit(this);
    m_sourceEdit->setPlaceholderText(QStringLiteral("Repository URL to merge from"));

    m_revisionRangeEdit = new QLineEdit(this);
    m_revisionRangeEdit->setPlaceholderText(QStringLiteral("Example: 10:20"));

    m_dryRunCheck = new QCheckBox(QStringLiteral("Dry run"), this);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("From URL:"), m_sourceEdit);
    form->addRow(QStringLiteral("Revision range:"), m_revisionRangeEdit);
    form->addRow(QString(), m_dryRunCheck);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &MergeDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void MergeDialog::setSourceUrl(const QString &url)
{
    m_sourceEdit->setText(url);
}

QString MergeDialog::sourceUrl() const
{
    return m_sourceEdit->text().trimmed();
}

QString MergeDialog::revisionRange() const
{
    return m_revisionRangeEdit->text().trimmed();
}

bool MergeDialog::dryRun() const
{
    return m_dryRunCheck->isChecked();
}

void MergeDialog::validateAndAccept()
{
    if (sourceUrl().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Merge"), QStringLiteral("Source URL is required."));
        return;
    }

    accept();
}
