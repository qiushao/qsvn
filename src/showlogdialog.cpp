#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QtWidgets/QSplitter>
#include "utils/log.h"
#include "utils/cmd.h"
#include "utils/stringutils.h"

#include "showlogdialog.h"

ShowLogDialog::ShowLogDialog(std::string uri) {
    setWindowTitle(QString::fromStdString("qsvn log " + uri));
    setMinimumSize(720, 480);

    mUri = uri;
    mStartRevision = "HEAD";
    mSearchPattern = "";

    mInputEdit = new QLineEdit(this);
    QObject::connect(mInputEdit, &QLineEdit::textChanged, this, &ShowLogDialog::onSearchTextChanged);

    //////////////////////////  search button setup
    mSearchButton = new QPushButton(this);
    mSearchButton->setText("search");
    QObject::connect(mSearchButton, &QPushButton::clicked, this, &ShowLogDialog::onSearchButtonClicked);

    ////////////////////////// next 100 button setup
    mNextButton = new QPushButton(this);
    mNextButton->setText("next 100");
    QObject::connect(mNextButton, &QPushButton::clicked, this, &ShowLogDialog::onNextButtonClicked);

    ////////////////////////// show all button setup
    mShowAllButton = new QPushButton(this);
    mShowAllButton->setText("show all");
    QObject::connect(mShowAllButton, &QPushButton::clicked, this, &ShowLogDialog::onShowAllButtonClicked);

    ////////////////////////// log list setup
    mLogListView = new QTableWidget(this);
    mLogListView->setColumnCount(4);
    QStringList logListHeader;
    logListHeader << "revision" << "auth" << "date" << "comment";
    mLogListView->setHorizontalHeaderLabels(logListHeader);
    mLogListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mLogListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mLogListView->horizontalHeader()->setStretchLastSection(true);
    QObject::connect(mLogListView, &QTableWidget::itemSelectionChanged, this, &ShowLogDialog::onLogListSelected);

    mMsgDetailView = new QTextEdit(this);
    mMsgDetailView->setReadOnly(true);

    /////////////////////////// change list setup
    mChangeListView = new QTableWidget(this);
    mChangeListView->setColumnCount(3);
    QStringList changedListHeader;
    changedListHeader << "action" << "type" << "path";
    mChangeListView->setHorizontalHeaderLabels(changedListHeader);
    mChangeListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mChangeListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mChangeListView->horizontalHeader()->setStretchLastSection(true);
    QObject::connect(mChangeListView, &QTableWidget::itemDoubleClicked, this, &ShowLogDialog::onChangedPathDoubleClick);

    /////////////////////////// layout setup
    //search layout
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(mInputEdit);
    topLayout->addWidget(mSearchButton);
    topLayout->addWidget(mNextButton);
    topLayout->addWidget(mShowAllButton);

    //log list layout
    QSplitter *splitterMain = new QSplitter(Qt::Vertical, this);
    splitterMain->addWidget(mLogListView);
    splitterMain->addWidget(mMsgDetailView);
    splitterMain->addWidget(mChangeListView);
    //上下比例为5：2
    //设置第0个 widget 的比例
    splitterMain->setStretchFactor(0, 5);
    //设置第1个 widget 的比例
    splitterMain->setStretchFactor(1, 1);
    splitterMain->setStretchFactor(2, 3);

    //main
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(splitterMain);

    setLayout(mainLayout);
    loadNextPage();
}

void ShowLogDialog::onSearchButtonClicked() {
    mSearchPattern = mInputEdit->text().toStdString();
    mSearchPattern = strTrim(mSearchPattern);
    for (int j = 0; j < mLogListView->rowCount(); ++j) {
        mLogListView->setRowHidden(j, true);
    }

    for (int i = 0; i < mAllLogEntries.size(); ++i) {
        LogEntry *entry = mAllLogEntries[i];
        if (isMatchSearchPattern(entry)) {
            mLogListView->setRowHidden(i, false);
        }
    }
}

void ShowLogDialog::onSearchTextChanged(const QString &text) {
    onSearchButtonClicked();
}

void ShowLogDialog::onNextButtonClicked() {
    std::string endRevision = mAllLogEntries[mAllLogEntries.size()-1]->revision;
    int revision = std::stoi(endRevision);
    mStartRevision = std::to_string(revision - 1);
    loadNextPage();
}

void ShowLogDialog::onShowAllButtonClicked() {
    mNextButton->setDisabled(true);
    loadAllPage();
}

void ShowLogDialog::onLogListSelected() {
    mChangeListView->clearContents();
    int row = mLogListView->currentRow();
    LogEntry *entry = mAllLogEntries[row];
    mChangeListView->setRowCount(entry->pathList.size());
    for (int i = 0; i < entry->pathList.size(); ++i) {
        QTableWidgetItem *actionItem = new QTableWidgetItem(QString::fromStdString(entry->pathList[i]->action));
        QTableWidgetItem *kindItem = new QTableWidgetItem(QString::fromStdString(entry->pathList[i]->kind));
        QTableWidgetItem *pathItem = new QTableWidgetItem(QString::fromStdString(entry->pathList[i]->path));
        if (!mSearchPattern.empty() && isContainIgnoreCase(entry->pathList[i]->path, mSearchPattern)) {
            pathItem->setTextColor(QColor(255, 0, 0));
        }
        mChangeListView->setItem(i, 0, actionItem);
        mChangeListView->setItem(i, 1, kindItem);
        mChangeListView->setItem(i, 2, pathItem);
    }
    mMsgDetailView->setText(QString::fromStdString(entry->msg));
}

void ShowLogDialog::onChangedPathDoubleClick(QTableWidgetItem *item) {
    int logRow = mLogListView->currentRow();
    LogEntry *entry = mAllLogEntries[logRow];
    int changedRow = mChangeListView->currentRow();
    Path *path = entry->pathList[changedRow];

    std::string repoRoot = mClient.getRepositoryRoot(mUri);
    std::string diffPath = repoRoot + path->path;

    int revision = std::stoi(entry->revision);
    std::string preRevision = std::to_string(revision - 1);

    cmd("svn diff -r%s:%s %s &", entry->revision.c_str(), preRevision.c_str(), diffPath.c_str());
}

void ShowLogDialog::loadPage(int limit) {
    std::vector<LogEntry *> logList = mClient.getLog(mUri, mStartRevision, "1", limit);
    addDataToView(logList);
}

void ShowLogDialog::loadNextPage() {
    loadPage(100);
}

void ShowLogDialog::loadAllPage() {
    loadPage(10000);
}

void ShowLogDialog::addDataToView(std::vector<LogEntry*> &logList) {
    int orgCount = mAllLogEntries.size();
    LogEntry *entry = nullptr;
    int row = 0;
    mLogListView->setRowCount(logList.size() + orgCount);
    for (int i = 0; i < logList.size(); i++) {
        entry = logList[i];
        row = i + orgCount;
        mLogListView->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(entry->revision)));
        mLogListView->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entry->author)));
        mLogListView->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(entry->date)));
        mLogListView->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(entry->msg)));
        mAllLogEntries.push_back(logList[i]);
        if (!isMatchSearchPattern(entry)) {
            mLogListView->setRowHidden(row, true);
        }
    }
    mLogListView->selectRow(orgCount);
}

bool ShowLogDialog::isMatchSearchPattern(LogEntry *entry) {
    if (mSearchPattern.empty()) {
        return true;
    }

    if (isContainIgnoreCase(entry->author, mSearchPattern)
        || isContainIgnoreCase(entry->date, mSearchPattern)
        || isContainIgnoreCase(entry->msg, mSearchPattern)
        || isContainIgnoreCase(entry->revision, mSearchPattern)) {
        return true;
    }

    for (int i = 0; i < entry->pathList.size(); ++i) {
        if (isContainIgnoreCase(entry->pathList[i]->path, mSearchPattern)) {
            return true;
        }
    }

    return false;
}