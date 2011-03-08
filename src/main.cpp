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
    bool bQuit = false;
    if ((app.argc() >= 2) && (0 == strcmp(app.argv()[1],"quit"))) {
        bQuit = true;
    }
    if (app.isRunning ()) {
        if (bQuit) {
            app.sendMessage ("quit");
        } else {
            app.sendMessage ("show");
            bQuit = true;
        }
    }
    if (bQuit == true) {
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
#elif defined(Q_WS_MAEMO_5)
    w.showFullScreen ();
#else
    w.show();
#endif

    int rv = app.exec();
    pw = NULL;
    return (rv);
}//main
