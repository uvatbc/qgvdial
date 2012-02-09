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

#include <iostream>
using namespace std;

QtMsgHandler    pOldHandler = NULL;
MainWindow     *pw = NULL;

QFile fLogfile;       //! Logfile
int   logLevel = 5;   //! Log level
int   logCounter = 0; //! Number of log entries since the last log flush

QStringList arrLogFiles;

void
qgv_LogFlush()
{
    if (logCounter) {
        logCounter = 0;
        fLogfile.flush ();
    }
}

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

    if ((level <= logLevel) && (NULL != pw)) {
        pw->log (dt, level, strLog);
    }

    strLog += '\n';
    if (level <= logLevel) {
        if (fLogfile.isOpen ()) {
            // Append it to the file
            fLogfile.write(strLog.toLatin1 ());

            ++logCounter;
            if (logCounter > 50) {
                qgv_LogFlush ();
            }
        }
    }

    if (NULL == pOldHandler) {
        if (QtFatalMsg == type) {
            abort();
        }
    } else {
        pOldHandler (type, strLog.toLatin1 ());
    }
}//myMessageOutput

static void
initLogging ()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strLogfile = osd.getAppDirectory ();
    strLogfile += QDir::separator ();
    strLogfile += "qgvdial.log";

    for (int i = 4; i >= 0; i--) {
        arrLogFiles.append (QString("%1.%2").arg(strLogfile).arg(i));
    }
    arrLogFiles.append (strLogfile);

    QFile::remove (arrLogFiles[0]);
    for (int i = 1; i < arrLogFiles.count (); i++) {
        if (QFile::exists (arrLogFiles[i])) {
            QFile::rename (arrLogFiles[i], arrLogFiles[i-1]);
        }
    }

    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::ReadWrite);

    pOldHandler = qInstallMsgHandler(myMessageOutput);
}//initLogging

static void
deinitLogging ()
{
    pw = NULL;
    fLogfile.close ();
}//deinitLogging

int
main (int argc, char *argv[])
{
#if defined(Q_WS_S60)
    MainApp::setAttribute (Qt::AA_S60DontConstructApplicationPanes);
#endif

    initLogging ();

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

    // Phonon needs the name of the application
    app.setApplicationName (APPLICATION_NAME);

    int rv = app.exec();
    deinitLogging ();
    return (rv);
}//main
