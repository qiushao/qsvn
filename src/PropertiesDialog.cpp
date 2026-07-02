#include "PropertiesDialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

PropertiesDialog::PropertiesDialog(SvnClient *svnClient, const QString &workingDirectory, const QString &targetPath, QWidget *parent)
    : QDialog(parent)
    , m_svnClient(svnClient)
    , m_workingDirectory(workingDirectory)
    , m_targetPath(targetPath)
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowTitle(QStringLiteral("Properties"));
    resize(760, 560);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({QStringLiteral("Name"), QStringLiteral("Value")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int currentRow) {
        showSelectedProperty(currentRow);
    });

    m_valueView = new QPlainTextEdit(this);
    m_valueView->setReadOnly(true);
    m_valueView->setMaximumHeight(160);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    auto *refreshButton = buttons->addButton(QStringLiteral("Refresh"), QDialogButtonBox::ActionRole);
    auto *setButton = buttons->addButton(QStringLiteral("Set..."), QDialogButtonBox::ActionRole);
    auto *deleteButton = buttons->addButton(QStringLiteral("Delete"), QDialogButtonBox::ActionRole);
    connect(refreshButton, &QPushButton::clicked, this, &PropertiesDialog::loadProperties);
    connect(setButton, &QPushButton::clicked, this, &PropertiesDialog::setProperty);
    connect(deleteButton, &QPushButton::clicked, this, &PropertiesDialog::deleteProperty);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_valueView);
    layout->addWidget(buttons);
}

void PropertiesDialog::refresh()
{
    loadProperties();
}

void PropertiesDialog::loadProperties()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("proplist"), QStringLiteral("--xml"), QStringLiteral("-v"), m_targetPath}, m_workingDirectory);
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Properties failed"), result.combinedOutput());
        return;
    }

    m_properties = m_svnClient->parsePropertiesXml(result.standardOutput);
    m_table->setRowCount(m_properties.size());
    for (int row = 0; row < m_properties.size(); ++row) {
        const SvnProperty &property = m_properties.at(row);
        m_table->setItem(row, 0, new QTableWidgetItem(property.name));
        m_table->setItem(row, 1, new QTableWidgetItem(property.value.simplified()));
    }

    if (!m_properties.isEmpty()) {
        m_table->selectRow(0);
        showSelectedProperty(0);
    } else {
        m_valueView->clear();
    }
}

void PropertiesDialog::setProperty()
{
    bool ok = false;
    const QString currentName = selectedPropertyName();
    const QString name = QInputDialog::getText(this,
                                               QStringLiteral("Set Property"),
                                               QStringLiteral("Property name:"),
                                               QLineEdit::Normal,
                                               currentName,
                                               &ok)
                             .trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }

    const QString value = QInputDialog::getMultiLineText(this,
                                                        QStringLiteral("Set Property"),
                                                        QStringLiteral("Property value:"),
                                                        name == currentName ? selectedPropertyValue() : QString(),
                                                        &ok);
    if (!ok) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("propset"), name, value, m_targetPath}, m_workingDirectory);
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Set Property failed"), result.combinedOutput());
        return;
    }

    loadProperties();
}

void PropertiesDialog::deleteProperty()
{
    const QString name = selectedPropertyName();
    if (name.isEmpty()) {
        return;
    }

    if (QMessageBox::question(this, QStringLiteral("Delete Property"), QStringLiteral("Delete property '%1'?").arg(name)) != QMessageBox::Yes) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const SvnResult result = m_svnClient->run({QStringLiteral("propdel"), name, m_targetPath}, m_workingDirectory);
    QApplication::restoreOverrideCursor();
    emit commandFinished(result);

    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("Delete Property failed"), result.combinedOutput());
        return;
    }

    loadProperties();
}

void PropertiesDialog::showSelectedProperty(int row)
{
    if (row < 0 || row >= m_properties.size()) {
        m_valueView->clear();
        return;
    }

    m_valueView->setPlainText(m_properties.at(row).value);
}

QString PropertiesDialog::selectedPropertyName() const
{
    const QList<QTableWidgetItem *> selectedItems = m_table->selectedItems();
    if (selectedItems.isEmpty()) {
        return QString();
    }

    return m_properties.at(selectedItems.first()->row()).name;
}

QString PropertiesDialog::selectedPropertyValue() const
{
    const QList<QTableWidgetItem *> selectedItems = m_table->selectedItems();
    if (selectedItems.isEmpty()) {
        return QString();
    }

    return m_properties.at(selectedItems.first()->row()).value;
}
