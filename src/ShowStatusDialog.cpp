//
// Created by mint on 11/5/18.
//
#include <QHeaderView>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QtWidgets/QSplitter>
#include "ShowStatusDialog.h"
#include "utils/log.h"

ShowStatusDialog::ShowStatusDialog(std::string uri) {
    setWindowTitle(QString::fromStdString("qsvn status " + uri));
    setMinimumSize(720, 480);

    mUri = uri;

    mInputEdit = new QTextEdit(this);

    mChangeListView = new QTableWidget(this);
    mChangeListView->setColumnCount(4);
    QStringList changedListHeader;
    changedListHeader << "select" << "action" << "type" << "path";
    mChangeListView->setHorizontalHeaderLabels(changedListHeader);
    mChangeListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mChangeListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mChangeListView->horizontalHeader()->setStretchLastSection(true);

    mCancelButton = new QPushButton(this);
    mCancelButton->setText("cancel");
    mCancelButton->setMaximumWidth(100);

    mRevertButton = new QPushButton(this);
    mRevertButton->setText("revert");
    mRevertButton->setMaximumWidth(100);

    mCommitButton = new QPushButton(this);
    mCommitButton->setText("commit");
    mCommitButton->setMaximumWidth(100);


    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(mCancelButton);
    bottomLayout->addWidget(mRevertButton);
    bottomLayout->addWidget(mCommitButton);
    bottomLayout->setMargin(10);

    //log list layout
    QSplitter *splitterMain = new QSplitter(Qt::Vertical, this);
    splitterMain->addWidget(mChangeListView);
    splitterMain->addWidget(mInputEdit);
    //上下比例为5：2
    //设置第0个 widget 的比例
    splitterMain->setStretchFactor(0, 5);
    //设置第1个 widget 的比例
    splitterMain->setStretchFactor(1, 1);

    //main
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(splitterMain);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);


}