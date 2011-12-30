#include "MainWindow.h"

QtMsgHandler pOldHandler = NULL;
MainWindow *pw = NULL;

void
myMessageOutput(QtMsgType type, const char *msg)
{
    QString strMsg = msg;
    int level = -1;
    switch (type) {
    case QtDebugMsg:
        level = 3;
        break;
    case QtWarningMsg:
        level = 2;
        break;
    case QtCriticalMsg:
        level = 1;
        break;
    case QtFatalMsg:
        level = 0;
    }

    if (NULL != pw) {
        pw->log (strMsg, level);
    }

    if (NULL == pOldHandler) {
        if (NULL == pw) {
            strMsg += "\n";
            fwrite (strMsg.toLatin1 (), strMsg.size (), 1, stderr);
        }

        if (QtFatalMsg == type) {
            abort();
        }
    } else {
        pOldHandler (type, strMsg.toLatin1 ());
    }
}//myMessageOutput

int
main (int argc, char **argv)
{
    pOldHandler = qInstallMsgHandler(myMessageOutput);

    QCoreApplication app(argc, argv);
    MainWindow w;
    pw = &w;

    int rv = app.exec ();
    pw = NULL;
    return rv;
}//main
