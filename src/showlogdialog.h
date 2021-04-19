#ifndef SHOWLOGDIALOG_H
#define SHOWLOGDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QtWidgets/QTextEdit>
#include "SVNClient.h"

class ShowLogDialog : public QDialog
{
public:
    ShowLogDialog(std::string uri);

private:
    void loadNextPage();
    void loadAllPage();
    void loadPage(int limit);
    void addDataToView(std::vector<LogEntry*> &logList);
    bool isMatchSearchPattern(LogEntry *entry);

private:
    QLineEdit *mInputEdit;
    QPushButton *mSearchButton;
    QPushButton *mNextButton;
    QPushButton *mShowAllButton;
    QTableWidget *mLogListView;
    QTextEdit *mMsgDetailView;
    QTableWidget *mChangeListView;

    SVNClient mClient;
    std::vector<LogEntry*> mAllLogEntries;
    std::string mUri;
    std::string mStartRevision;
    std::string mSearchPattern;

    void onSearchButtonClicked();
    void onNextButtonClicked();
    void onShowAllButtonClicked();
    void onLogListSelected();
    void onChangedPathDoubleClick(QTableWidgetItem *item);
    void onSearchTextChanged(const QString &text);
};

#endif // SHOWLOGDIALOG_H
