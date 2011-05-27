/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

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
#if defined(Q_WS_S60)
    MainApp::setAttribute (Qt::AA_S60DontConstructApplicationPanes);
#endif

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

#if defined(Q_OS_SYMBIAN)
    app.setQuitOnLastWindowClosed (true);
#else
    app.setQuitOnLastWindowClosed (false);
#endif

#if MOBILE_OS
    w.showFullScreen ();
#else
    w.show();
#endif

    int rv = app.exec();
    pw = NULL;
    return (rv);
}//main
