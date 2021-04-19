#include <QApplication>
#include <unistd.h>
#include "showlogdialog.h"
#include "ShowStatusDialog.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/stringutils.h"
#include "SVNClient.h"

void usage() {
    printf("usage :\n");
    printf("qsvn log [path] : show log of path, if not define path, use current dir instead\n");
    printf("qsvn diff [path] : show diff of path, if not define path, use current dir instead\n");
    printf("\n");
}

static std::string SUB_CMD_LOG = "log";
static std::string SUB_CMD_STATUS = "st";

int main(int argc, char *argv[])
{
    std::string uri;
    if (argc < 2) {
        //first arg is execute path
        LOG("no sub command define, show usage");
        usage();
        return -1;
    }

    QApplication a(argc, argv);
    QDialog *dialog = nullptr;

    std::string subcmd(argv[1]);

    if (argc < 3) {
        LOG("no path define, use current dir");
        char buf[256];
        getcwd(buf,sizeof(buf));
        uri = buf;
    } else {
        uri = argv[2];
    }

    if(strEqual(subcmd, SUB_CMD_LOG)) {
        dialog = new ShowLogDialog(uri);
        dialog->showMaximized();
    } else if (strEqual(subcmd, SUB_CMD_STATUS)) {
        dialog = new ShowStatusDialog(uri);
        dialog->showMaximized();
    } else {
        LOG("no such sub command");
        usage();
        return -1;
    }

    return a.exec();
}
