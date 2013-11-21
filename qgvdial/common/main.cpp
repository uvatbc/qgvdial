/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#include "MainWindow.h"
#include "Lib.h"

#include <iostream>
using namespace std;

QtMsgHandler    pOldHandler = NULL;
MainWindow     *win = NULL;

QFile fLogfile;       //! Logfile
int   logLevel = 5;   //! Log level
int   logCounter = 0; //! Number of log entries since the last log flush
#define LOG_FLUSH_LEVEL 0

QStringList g_arrLogFiles;

const char *ignoreMsgs[] = {
    "MPanRecognizerTouch",
    "QGLWindowSurface",
    "brokenTexSubImage",
    "FullClearOnEveryFrame",
    "hijackWindow"
};

void
qgv_LogFlush()
{
    if (logCounter) {
        logCounter = 0;
        fLogfile.flush ();
        cout.flush();
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
        break;
    }

    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg);

    // Ignore some log messages.
    for (quint16 i = 0; i < COUNT_OF(ignoreMsgs); i++) {
        if (strLog.contains(ignoreMsgs[i])) {
            return;
        }
    }

    // Send to standard output.
    // I'm not using endl here because endl causes flushes
    cout << strLog.toAscii().constData() << "\n";

    if ((level <= logLevel) && (NULL != win)) {
        win->log(dt, level, strLog);
    }

    strLog += '\n';
    if (level <= logLevel) {
        if (fLogfile.isOpen ()) {
            // Append it to the file
            fLogfile.write(strLog.toLatin1 ());

            ++logCounter;
            if (logCounter > LOG_FLUSH_LEVEL) {
                qgv_LogFlush ();
            }
        }
    }

    if (QtFatalMsg == type) {
        abort();
    }
}//myMessageOutput

static void
initLogging ()
{
    Lib &lib = Lib::ref ();
    QString strLogfile = lib.getLogsDir ();
    strLogfile += QDir::separator ();
    strLogfile += "qgvdial-startup.log";

    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::ReadWrite);

    pOldHandler = qInstallMsgHandler(myMessageOutput);
}//initLogging

static void
initLogRotate()
{
    Lib &lib = Lib::ref ();
    QString strLogfile = lib.getLogsDir ();
    strLogfile += QDir::separator ();
    strLogfile += "qgvdial.log";

    for (int i = 4; i >= 0; i--) {
        g_arrLogFiles.append (QString("%1.%2").arg(strLogfile).arg(i));
    }
    g_arrLogFiles.append (strLogfile);

    QFile::remove (g_arrLogFiles[0]);
    for (int i = 1; i < g_arrLogFiles.count (); i++) {
        if (QFile::exists (g_arrLogFiles[i])) {
            QFile::rename (g_arrLogFiles[i], g_arrLogFiles[i-1]);
        }
    }

    fLogfile.close ();
    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::ReadWrite);
}//initLogRotate

static void
deinitLogging ()
{
    win = NULL;
    fLogfile.close ();
}//deinitLogging

Q_DECL_EXPORT int
main(int argc, char *argv[])
{
    QCoreApplication *app = createAppObject(argc, argv);
    if (NULL == app) {
        // For whatever reason (not necessarily a lack of memory), the fn has
        // told us that creating this app is NOT possible. GTFO NOW!
        return (-1);
    }

    // Phonon needs this. Doesn't hurt even if we don't use Phonon.
    app->setApplicationName ("qgvdial");

    initLogging ();
    initLogRotate ();

    win = new MainWindow(app);
    win->init();

    int rv = app->exec();

    // Safety...
    MainWindow *temp = win;
    deinitLogging ();

    delete temp;
    delete app;
    return rv;
}
