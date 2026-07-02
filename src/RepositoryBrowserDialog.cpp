#include "RepositoryBrowserDialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

RepositoryBrowserDialog::RepositoryBrowserDialog(const SvnClient *svnClient, QWidget *parent)
    : QDialog(parent)
    , m_svnClient(svnClient)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    setWindowTitle(QStringLiteral("Repository Browser"));
    resize(760, 560);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText(QStringLiteral("https://example.com/svn/project/trunk"));
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &RepositoryBrowserDialog::listUrl);

    auto *goButton = new QPushButton(QStringLiteral("Go"), this);
    connect(goButton, &QPushButton::clicked, this, &RepositoryBrowserDialog::listUrl);

    auto *upButton = new QPushButton(QStringLiteral("Up"), this);
    connect(upButton, &QPushButton::clicked, this, &RepositoryBrowserDialog::goUp);

    auto *urlLayout = new QHBoxLayout;
    urlLayout->addWidget(m_urlEdit, 1);
    urlLayout->addWidget(goButton);
    urlLayout->addWidget(upButton);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({QStringLiteral("Name"), QStringLiteral("Kind")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_table, &QTableWidget::itemDoubleClicked, this, &RepositoryBrowserDialog::openSelected);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *newFolderButton = buttons->addButton(QStringLiteral("New Folder..."), QDialogButtonBox::ActionRole);
    auto *copyButton = buttons->addButton(QStringLiteral("Copy To..."), QDialogButtonBox::ActionRole);
    auto *renameButton = buttons->addButton(QStringLiteral("Rename..."), QDialogButtonBox::ActionRole);
    auto *deleteButton = buttons->addButton(QStringLiteral("Delete"), QDialogButtonBox::ActionRole);
    m_checkoutButton = buttons->addButton(QStringLiteral("Checkout..."), QDialogButtonBox::AcceptRole);
    connect(newFolderButton, &QPushButton::clicked, this, &RepositoryBrowserDialog::createFolder);
    connect(copyButton, &QPushButton::clicked, this, &RepositoryBrowserDialog::copySelected);
    connect(renameButton, &QPushButton::clicked, this, &RepositoryBrowserDialog::renameSelected);
    connect(deleteButton, &QPushButton::clicked, this, &RepositoryBrowserDialog::deleteSelected);
    connect(m_checkoutButton, &QPushButton::clicked, this, &RepositoryBrowserDialog::acceptCheckout);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(urlLayout);
    layout->addWidget(m_table, 1);
    layout->addWidget(buttons);
}

QString RepositoryBrowserDialog::checkoutUrl() const
{
    return m_checkoutUrl;
}

void RepositoryBrowserDialog::setRepositoryUrl(const QString &url)
{
    m_urlEdit->setText(url);
}

void RepositoryBrowserDialog::listUrl()
{
    const QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Repository Browser"), QStringLiteral("Repository URL is required."));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("list"), url});
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("List failed"), result.combinedOutput());
        return;
    }

    const QStringList lines = result.standardOutput.split('\n', Qt::SkipEmptyParts);
    m_table->setRowCount(lines.size());
    for (int row = 0; row < lines.size(); ++row) {
        QString name = lines.at(row).trimmed();
        const bool isDirectory = name.endsWith('/');
        if (isDirectory) {
            name.chop(1);
        }

        auto *nameItem = new QTableWidgetItem(name);
        nameItem->setData(Qt::UserRole, isDirectory);
        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, new QTableWidgetItem(isDirectory ? QStringLiteral("Directory") : QStringLiteral("File")));
    }
}

void RepositoryBrowserDialog::goUp()
{
    const QString parent = parentUrl(m_urlEdit->text().trimmed());
    if (!parent.isEmpty() && parent != m_urlEdit->text().trimmed()) {
        m_urlEdit->setText(parent);
        listUrl();
    }
}

void RepositoryBrowserDialog::openSelected()
{
    const QList<QTableWidgetItem *> items = m_table->selectedItems();
    if (items.isEmpty()) {
        return;
    }

    const int row = items.first()->row();
    const auto *nameItem = m_table->item(row, 0);
    if (nameItem == nullptr || !nameItem->data(Qt::UserRole).toBool()) {
        return;
    }

    m_urlEdit->setText(appendPath(m_urlEdit->text().trimmed(), nameItem->text()));
    listUrl();
}

void RepositoryBrowserDialog::createFolder()
{
    const QString currentUrl = m_urlEdit->text().trimmed();
    if (currentUrl.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Repository Browser"), QStringLiteral("Repository URL is required."));
        return;
    }

    bool ok = false;
    const QString name = QInputDialog::getText(this,
                                               QStringLiteral("New Folder"),
                                               QStringLiteral("Folder name:"),
                                               QLineEdit::Normal,
                                               QString(),
                                               &ok)
                             .trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }

    const QString message = QInputDialog::getMultiLineText(this,
                                                          QStringLiteral("New Folder"),
                                                          QStringLiteral("Log message:"),
                                                          QStringLiteral("Create folder ") + name,
                                                          &ok)
                                .trimmed();
    if (!ok || message.isEmpty()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("mkdir"), appendPath(currentUrl, name), QStringLiteral("-m"), message});
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("New Folder failed"), result.combinedOutput());
        return;
    }

    listUrl();
}

void RepositoryBrowserDialog::copySelected()
{
    const QString sourceUrl = currentOrSelectedUrl();
    if (sourceUrl.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Copy"), QStringLiteral("Repository URL is required."));
        return;
    }

    QString defaultTarget = sourceUrl;
    while (defaultTarget.endsWith('/')) {
        defaultTarget.chop(1);
    }
    defaultTarget.append(QStringLiteral("-copy"));

    bool ok = false;
    const QString targetUrl = QInputDialog::getText(this,
                                                    QStringLiteral("Copy To"),
                                                    QStringLiteral("Target URL:"),
                                                    QLineEdit::Normal,
                                                    defaultTarget,
                                                    &ok)
                                  .trimmed();
    if (!ok || targetUrl.isEmpty() || targetUrl == sourceUrl) {
        return;
    }

    const QString message = QInputDialog::getMultiLineText(this,
                                                          QStringLiteral("Copy To"),
                                                          QStringLiteral("Log message:"),
                                                          QStringLiteral("Copy ") + sourceUrl,
                                                          &ok)
                                .trimmed();
    if (!ok || message.isEmpty()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("copy"), sourceUrl, targetUrl, QStringLiteral("-m"), message});
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Copy failed"), result.combinedOutput());
        return;
    }

    listUrl();
}

void RepositoryBrowserDialog::deleteSelected()
{
    const QString url = selectedUrl();
    if (url.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Delete"), QStringLiteral("Select a repository item first."));
        return;
    }

    if (QMessageBox::question(this, QStringLiteral("Delete"), QStringLiteral("Delete '%1'?").arg(url)) != QMessageBox::Yes) {
        return;
    }

    bool ok = false;
    const QString message = QInputDialog::getMultiLineText(this,
                                                          QStringLiteral("Delete"),
                                                          QStringLiteral("Log message:"),
                                                          QStringLiteral("Delete ") + selectedName(),
                                                          &ok)
                                .trimmed();
    if (!ok || message.isEmpty()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("delete"), url, QStringLiteral("-m"), message});
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Delete failed"), result.combinedOutput());
        return;
    }

    listUrl();
}

void RepositoryBrowserDialog::renameSelected()
{
    const QString url = selectedUrl();
    const QString name = selectedName();
    if (url.isEmpty() || name.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Rename"), QStringLiteral("Select a repository item first."));
        return;
    }

    bool ok = false;
    const QString newName = QInputDialog::getText(this,
                                                  QStringLiteral("Rename"),
                                                  QStringLiteral("New name:"),
                                                  QLineEdit::Normal,
                                                  name,
                                                  &ok)
                                .trimmed();
    if (!ok || newName.isEmpty() || newName == name) {
        return;
    }

    const QString message = QInputDialog::getMultiLineText(this,
                                                          QStringLiteral("Rename"),
                                                          QStringLiteral("Log message:"),
                                                          QStringLiteral("Rename ") + name + QStringLiteral(" to ") + newName,
                                                          &ok)
                                .trimmed();
    if (!ok || message.isEmpty()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("move"), url, appendPath(parentUrl(url), newName), QStringLiteral("-m"), message});
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Rename failed"), result.combinedOutput());
        return;
    }

    listUrl();
}

void RepositoryBrowserDialog::acceptCheckout()
{
    const QString url = currentOrSelectedUrl();
    if (url.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Repository Browser"), QStringLiteral("Repository URL is required."));
        return;
    }

    m_checkoutUrl = url;
    accept();
}

QString RepositoryBrowserDialog::selectedUrl() const
{
    const QString name = selectedName();
    if (name.isEmpty()) {
        return QString();
    }

    return appendPath(m_urlEdit->text().trimmed(), name);
}

QString RepositoryBrowserDialog::selectedName() const
{
    const QList<QTableWidgetItem *> items = m_table->selectedItems();
    if (items.isEmpty()) {
        return QString();
    }

    const int row = items.first()->row();
    const auto *nameItem = m_table->item(row, 0);
    return nameItem != nullptr ? nameItem->text() : QString();
}

QString RepositoryBrowserDialog::currentOrSelectedUrl() const
{
    const QList<QTableWidgetItem *> items = m_table->selectedItems();
    if (items.isEmpty()) {
        return m_urlEdit->text().trimmed();
    }

    const int row = items.first()->row();
    const auto *nameItem = m_table->item(row, 0);
    if (nameItem == nullptr) {
        return m_urlEdit->text().trimmed();
    }

    return appendPath(m_urlEdit->text().trimmed(), nameItem->text());
}

QString RepositoryBrowserDialog::appendPath(const QString &url, const QString &entry) const
{
    QString base = url;
    if (!base.endsWith('/')) {
        base.append('/');
    }
    return base + entry;
}

QString RepositoryBrowserDialog::parentUrl(const QString &url) const
{
    QString value = url;
    while (value.endsWith('/')) {
        value.chop(1);
    }

    const int schemeIndex = value.indexOf(QStringLiteral("://"));
    const int minimumSlash = schemeIndex >= 0 ? schemeIndex + 3 : 0;
    const int slash = value.lastIndexOf('/');
    if (slash <= minimumSlash) {
        return url;
    }

    return value.left(slash);
}
