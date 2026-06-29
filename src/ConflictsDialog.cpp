#include "ConflictsDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ConflictsDialog::ConflictsDialog(const QVector<SvnStatus> &conflicts, const QString &workingCopy, QWidget *parent)
    : QDialog(parent)
    , m_conflicts(conflicts)
    , m_workingCopy(workingCopy)
{
    setWindowTitle(QStringLiteral("Conflicts"));
    resize(760, 480);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({QStringLiteral("Status"), QStringLiteral("Path")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setRowCount(m_conflicts.size());

    for (int row = 0; row < m_conflicts.size(); ++row) {
        m_table->setItem(row, 0, new QTableWidgetItem(m_conflicts.at(row).code));
        m_table->setItem(row, 1, new QTableWidgetItem(m_conflicts.at(row).path));
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *editButton = buttons->addButton(QStringLiteral("Edit Conflict"), QDialogButtonBox::ActionRole);
    auto *resolveButton = buttons->addButton(QStringLiteral("Resolve..."), QDialogButtonBox::ActionRole);
    connect(editButton, &QPushButton::clicked, this, &ConflictsDialog::editSelected);
    connect(resolveButton, &QPushButton::clicked, this, &ConflictsDialog::resolveSelected);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_table, 1);
    layout->addWidget(buttons);

    if (!m_conflicts.isEmpty()) {
        m_table->selectRow(0);
    }
}

void ConflictsDialog::editSelected()
{
    const QString path = selectedPath();
    if (!path.isEmpty()) {
        emit editConflictRequested(path);
    }
}

void ConflictsDialog::resolveSelected()
{
    const QString path = selectedPath();
    if (path.isEmpty()) {
        return;
    }

    const QStringList choices = {
        QStringLiteral("working"),
        QStringLiteral("mine-conflict"),
        QStringLiteral("theirs-conflict"),
        QStringLiteral("base"),
    };

    bool ok = false;
    const QString choice = QInputDialog::getItem(this,
                                                QStringLiteral("Resolve"),
                                                QStringLiteral("Accept:"),
                                                choices,
                                                0,
                                                false,
                                                &ok);
    if (ok && !choice.isEmpty()) {
        emit resolveRequested(path, choice);
    }
}

QString ConflictsDialog::selectedPath() const
{
    const QList<QTableWidgetItem *> selectedItems = m_table->selectedItems();
    if (selectedItems.isEmpty()) {
        return QString();
    }

    const int row = selectedItems.first()->row();
    const QString path = m_conflicts.at(row).path;
    if (QFileInfo(path).isAbsolute()) {
        return path;
    }

    return QDir(m_workingCopy).absoluteFilePath(path);
}
