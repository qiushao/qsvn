#include "CommitDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>

CommitDialog::CommitDialog(const QVector<SvnStatus> &statuses, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowTitle(QStringLiteral("Commit"));
    resize(640, 520);

    m_messageEdit = new QTextEdit(this);
    m_messageEdit->setPlaceholderText(QStringLiteral("Commit message"));
    m_messageEdit->setMinimumHeight(120);

    m_pathList = new QListWidget(this);
    for (const SvnStatus &status : statuses) {
        auto *item = new QListWidgetItem(status.code + QStringLiteral("  ") + status.path, m_pathList);
        item->setData(Qt::UserRole, status.path);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (status.code.startsWith('?')) {
            item->setCheckState(Qt::Unchecked);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        } else {
            item->setCheckState(Qt::Checked);
        }
    }

    if (statuses.isEmpty()) {
        auto *item = new QListWidgetItem(QStringLiteral("No local changes"), m_pathList);
        item->setFlags(Qt::NoItemFlags);
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &CommitDialog::validateAndAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(QStringLiteral("Message:"), this));
    layout->addWidget(m_messageEdit);
    layout->addWidget(new QLabel(QStringLiteral("Changes:"), this));
    layout->addWidget(m_pathList, 1);
    layout->addWidget(buttons);
}

QString CommitDialog::message() const
{
    return m_messageEdit->toPlainText().trimmed();
}

QStringList CommitDialog::selectedPaths() const
{
    QStringList paths;
    for (int i = 0; i < m_pathList->count(); ++i) {
        const QListWidgetItem *item = m_pathList->item(i);
        if (item->checkState() == Qt::Checked) {
            paths.push_back(item->data(Qt::UserRole).toString());
        }
    }
    return paths;
}

void CommitDialog::validateAndAccept()
{
    if (message().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Commit"), QStringLiteral("Commit message is required."));
        return;
    }
    if (selectedPaths().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Commit"), QStringLiteral("Select at least one versioned path."));
        return;
    }
    accept();
}
