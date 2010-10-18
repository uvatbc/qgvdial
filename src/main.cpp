#include "MainApp.h"
#include "MainWindow.h"
#include "Singletons.h"

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
main (int argc, char *argv[])
{
    pOldHandler = qInstallMsgHandler(myMessageOutput);

    MainApp app(argc, argv);
    if (app.isRunning ()) {
        app.sendMessage ("show");
        return (0);
    }

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.init ();

    MainWindow w;
    pw = &w;
    app.setActivationWindow (&w);
    app.setQuitOnLastWindowClosed (false);

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    int rv = app.exec();
    pw = NULL;
    return (rv);
}//main
