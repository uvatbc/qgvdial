/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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

#include "Srv.h"

#include <iostream>
using namespace std;

QFile fLogfile;       //! Logfile
int   logCounter = 0; //! Number of log entries since the last log flush
#define LOG_FLUSH_LEVEL 0

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
    QString strLog = QString("%1 : [%2] : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg);

    // Send to standard output.
    // I'm not using endl here because endl causes flushes
    cout << strLog.toLatin1().constData() << "\n";

    strLog += '\n';
    if (fLogfile.isOpen ()) {
        // Append it to the file
        fLogfile.write(strLog.toLatin1 ());

        ++logCounter;
        if (logCounter > LOG_FLUSH_LEVEL) {
            fLogfile.flush ();
            cout.flush();
        }
    }

    if (QtFatalMsg == type) {
        abort();
    }
}//myMessageOutput

void
initLogs()
{
    QString strLogfile = QDir::currentPath() + "/logs/qt4srv.log";

    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::ReadWrite | QIODevice::Truncate);

    qInstallMsgHandler(myMessageOutput);
}//initLogs

Q_DECL_EXPORT int
main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);
   MainObject o;

   initLogs ();

   int rv = app.exec();
   return rv;
}//main

