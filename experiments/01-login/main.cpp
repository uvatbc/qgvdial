#include "mainwindow.h"

#include <iostream>
using namespace std;

QtMsgHandler pOldHandler = NULL;
MainWindow *pw = NULL;

void
myMessageOutput(QtMsgType type, const char *msg)
{
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

    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg);

    // Send to standard output
    cout << strLog.toStdString () << endl;

    if (pw) {
        pw->log(strLog);
    }

    if (NULL == pOldHandler) {
        if (QtFatalMsg == type) {
            abort();
        }
    } else {
        pOldHandler (type, strLog.toLatin1 ());
    }
}//myMessageOutput

int
main(int argc, char *argv[])
{
    pOldHandler = qInstallMsgHandler(myMessageOutput);

    QApplication app(argc, argv);
    MainWindow mainWindow;
    pw = &mainWindow;

    mainWindow.setOrientation(MainWindow::ScreenOrientationAuto);
    mainWindow.showExpanded();

    int rv = app.exec();
    pw = NULL;
    return rv;
}//main
