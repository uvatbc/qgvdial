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

    QRegExp regex("^\"(.*)\"\\s*");
    if (strLog.indexOf (regex) != -1) {
        strLog = regex.cap (1);
    }

    if (strLog.contains ("password", Qt::CaseInsensitive)) {
        strLog = QString("%1 : %2 : %3")
                    .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                    .arg(level)
                    .arg("Log message with password blocked");
    }

    if ((level <= logLevel) && (NULL != pw)) {
        pw->log (strLog);
    }

    strLog += '\n';
    if (level <= logLevel) {
        if (fLogfile.isOpen ()) {
            // Append it to the file
            fLogfile.write(strLog.toLatin1 ());

            ++logCounter;
            if (logCounter > 100) {
                logCounter = 0;
                fLogfile.flush ();
            }
        }
    }

    if (NULL == pOldHandler) {
        if (NULL == pw) {
            fwrite (strLog.toLatin1 (), strLog.size (), 1, stderr);
        }

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
    QString strLog0 = strLogfile + ".0";
    QString strLog1 = strLogfile + ".1";
    QString strLog2 = strLogfile + ".2";
    QString strLog3 = strLogfile + ".3";
    QString strLog4 = strLogfile + ".4";

    QFile::remove (strLog4);
    if (QFile::exists (strLog3)) {
        QFile::rename (strLog3, strLog4);
    }
    if (QFile::exists (strLog2)) {
        QFile::rename (strLog2, strLog3);
    }
    if (QFile::exists (strLog1)) {
        QFile::rename (strLog1, strLog2);
    }
    if (QFile::exists (strLog0)) {
        QFile::rename (strLog0, strLog1);
    }
    if (QFile::exists (strLogfile)) {
        QFile::rename (strLogfile, strLog0);
    }

    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::WriteOnly | QIODevice::Append);
    fLogfile.seek (fLogfile.size ());

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
