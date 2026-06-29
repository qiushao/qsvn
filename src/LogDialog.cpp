#include "LogDialog.h"

#include <QBrush>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

LogDialog::LogDialog(const QVector<SvnLogEntry> &entries, QWidget *parent)
    : QDialog(parent)
    , m_entries(entries)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowTitle(QStringLiteral("Log"));
    resize(920, 680);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(QStringLiteral("Filter by author, message, or path..."));
    connect(m_filterEdit, &QLineEdit::textChanged, this, &LogDialog::applyFilter);

    m_entryTable = new QTableWidget(this);
    m_entryTable->setColumnCount(4);
    m_entryTable->setHorizontalHeaderLabels({
        QStringLiteral("Revision"),
        QStringLiteral("Author"),
        QStringLiteral("Date"),
        QStringLiteral("Message"),
    });
    m_entryTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_entryTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_entryTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_entryTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_entryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_entryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_entryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_entryTable->setRowCount(m_entries.size());

    for (int row = 0; row < m_entries.size(); ++row) {
        const SvnLogEntry &entry = m_entries.at(row);
        m_entryTable->setItem(row, 0, new QTableWidgetItem(entry.revision));
        m_entryTable->setItem(row, 1, new QTableWidgetItem(entry.author));
        m_entryTable->setItem(row, 2, new QTableWidgetItem(entry.date));
        m_entryTable->setItem(row, 3, new QTableWidgetItem(entry.message.simplified()));
    }
    connect(m_entryTable, &QTableWidget::currentCellChanged, this, [this](int currentRow) {
        showEntry(currentRow);
    });
    connect(m_filterEdit, &QLineEdit::textChanged, this, [this] {
        const int row = currentEntryRow();
        if (row >= 0) {
            showEntry(row);
        }
    });

    m_messageView = new QPlainTextEdit(this);
    m_messageView->setReadOnly(true);
    m_messageView->setMaximumHeight(120);

    m_pathTable = new QTableWidget(this);
    m_pathTable->setColumnCount(1);
    m_pathTable->setHorizontalHeaderLabels({QStringLiteral("Changed paths")});
    m_pathTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pathTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pathTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *showDiffButton = buttons->addButton(QStringLiteral("Show Diff"), QDialogButtonBox::ActionRole);
    auto *blameButton = buttons->addButton(QStringLiteral("Blame at Revision"), QDialogButtonBox::ActionRole);
    connect(showDiffButton, &QPushButton::clicked, this, &LogDialog::requestRevisionDiff);
    connect(blameButton, &QPushButton::clicked, this, &LogDialog::requestRevisionBlame);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_filterEdit);
    layout->addWidget(m_entryTable, 3);
    layout->addWidget(m_messageView);
    layout->addWidget(m_pathTable, 2);
    layout->addWidget(buttons);

    if (!m_entries.isEmpty()) {
        m_entryTable->selectRow(0);
        showEntry(0);
    }
}

void LogDialog::showEntry(int row)
{
    if (row < 0 || row >= m_entries.size()) {
        m_messageView->clear();
        m_pathTable->setRowCount(0);
        return;
    }

    const SvnLogEntry &entry = m_entries.at(row);
    m_messageView->setPlainText(entry.message);

    const QString keyword = m_filterEdit->text().trimmed().toLower();
    const QColor highlightColor = QColor(Qt::yellow).lighter(150);

    m_pathTable->setRowCount(entry.changedPaths.size());
    for (int pathRow = 0; pathRow < entry.changedPaths.size(); ++pathRow) {
        auto *item = new QTableWidgetItem(entry.changedPaths.at(pathRow));
        if (!keyword.isEmpty() && entry.changedPaths.at(pathRow).toLower().contains(keyword)) {
            item->setBackground(highlightColor);
        }
        m_pathTable->setItem(pathRow, 0, item);
    }
}

void LogDialog::requestRevisionDiff()
{
    const int row = currentEntryRow();
    if (row < 0 || row >= m_entries.size()) {
        return;
    }

    emit revisionDiffRequested(m_entries.at(row).revision);
}

void LogDialog::requestRevisionBlame()
{
    const int row = currentEntryRow();
    if (row < 0 || row >= m_entries.size()) {
        return;
    }

    const QString repositoryPath = repositoryPathFromChangedPath(currentChangedPath());
    if (repositoryPath.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Blame at Revision"), QStringLiteral("Select a changed path first."));
        return;
    }

    emit revisionBlameRequested(m_entries.at(row).revision, repositoryPath);
}

int LogDialog::currentEntryRow() const
{
    const QList<QTableWidgetItem *> selectedItems = m_entryTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return -1;
    }

    return selectedItems.first()->row();
}

QString LogDialog::currentChangedPath() const
{
    const QList<QTableWidgetItem *> selectedItems = m_pathTable->selectedItems();
    if (selectedItems.isEmpty()) {
        return QString();
    }

    return selectedItems.first()->text();
}

void LogDialog::applyFilter()
{
    const QString keyword = m_filterEdit->text().trimmed().toLower();

    for (int row = 0; row < m_entries.size(); ++row) {
        const SvnLogEntry &entry = m_entries.at(row);

        if (keyword.isEmpty()) {
            m_entryTable->setRowHidden(row, false);
            continue;
        }

        bool matches = entry.author.toLower().contains(keyword) ||
                       entry.message.toLower().contains(keyword);

        if (!matches) {
            for (const QString &path : entry.changedPaths) {
                if (path.toLower().contains(keyword)) {
                    matches = true;
                    break;
                }
            }
        }

        m_entryTable->setRowHidden(row, !matches);
    }
}

QString LogDialog::repositoryPathFromChangedPath(const QString &changedPath) const
{
    const int separator = changedPath.indexOf(QStringLiteral("  "));
    if (separator < 0) {
        return changedPath.trimmed();
    }

    return changedPath.mid(separator + 2).trimmed();
}
