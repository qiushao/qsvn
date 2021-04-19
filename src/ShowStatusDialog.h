//
// Created by mint on 11/5/18.
//

#ifndef QSVN_DIFFDIALOG_H
#define QSVN_DIFFDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QtWidgets/QTextEdit>
#include "SVNClient.h"

class ShowStatusDialog : public QDialog {
public:
    ShowStatusDialog(std::string uri);

private:
    QTextEdit *mInputEdit;
    QTableWidget *mChangeListView;
    QPushButton *mCancelButton;
    QPushButton *mRevertButton;
    QPushButton *mCommitButton;

    std::string mUri;
    SVNClient mClient;

};


#endif //QSVN_DIFFDIALOG_H
