#include "MainWindow.h"

#include <iostream>
using namespace std;

QtMsgHandler pOldHandler = NULL;
MainWindow *pw = NULL;
QStringList arrLogFiles;

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

    QRegExp regex("^\"(.*)\"\\s*");
    if (strMsg.indexOf (regex) != -1) {
        strMsg = regex.cap (1);
    }

    QDateTime dt = QDateTime::currentDateTime ();
    strMsg = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(strMsg);
    cout << strMsg.toStdString() << endl;

    if (QtFatalMsg == type) {
        abort();
    }
}//myMessageOutput

static void
initLogging ()
{
    pOldHandler = qInstallMsgHandler(myMessageOutput);
}//initLogging

static void
deinitLogging ()
{
    qInstallMsgHandler(pOldHandler);
    pw = NULL;
    pOldHandler = NULL;
}//deinitLogging

int
main (int argc, char **argv)
{
    initLogging ();

    QApplication app(argc, argv);
    MainWindow w;
    pw = &w;

    int rv = app.exec ();
    deinitLogging ();
    return rv;
}//main
