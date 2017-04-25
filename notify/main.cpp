/*
qgvnotify is a cross platform Google Voice Notification tool
Copyright (C) 2009-2017  Yuvraaj Kelkar

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
#include <iostream>
using namespace std;

QStringList arrLogFiles;
QFile fLogfile;       //! Logfile
int   logCounter = 0; //! Number of log entries since the last log flush
QString strLogfile;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
QtMessageHandler pOldHandler = NULL;
#else
QtMsgHandler     pOldHandler = NULL;
#endif

void
qgv_LogFlush()
{
}//qgv_LogFlush

void
myMessageOutput(QtMsgType type, const char *msg)
{
    QString strMsg = msg;
    int level = -1;
    switch (type) {
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
    case QtInfoMsg:
        level = 4;
        break;
#endif
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

    QDateTime dt = QDateTime::currentDateTime();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg)
                   + "\n";

    // Send to standard output
    cout << strLog.toStdString();

    if (fLogfile.isOpen()) {
        // Append it to the file
        fLogfile.write(strLog.toLatin1());
    }

    if (QtFatalMsg == type) {
        abort();
    }
}//myMessageOutput

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
void
myQt5MessageOutput(QtMsgType type, const QMessageLogContext & /*ctx*/,
                   const QString &msg)
{
    myMessageOutput(type, msg.toLatin1().constData());
}//myQt5MessageOutput
#endif

QString
baseDir()
{
    QString strBasedir = QDir::homePath();
    QDir baseDir(strBasedir);
    if (!baseDir.exists(".qgvdial")) {
        baseDir.mkdir(".qgvdial");
    }
    strBasedir += QDir::separator();
    strBasedir += ".qgvdial";
    return strBasedir;
}//baseDir

void
initLogging(const QString &userIni = QString())
{

    if (userIni.isEmpty()) {
        strLogfile = baseDir();
        strLogfile += QDir::separator();
        strLogfile += "notify.log";
    } else {
        strLogfile = userIni + ".log";
    }

    arrLogFiles.clear();
    for (int i = 4; i >= 0; i--) {
        arrLogFiles.append(QString("%1.%2").arg(strLogfile).arg(i));
    }
    arrLogFiles.append(strLogfile);

    QFile::remove(arrLogFiles[0]);
    for (int i = 1; i < arrLogFiles.count(); i++) {
        if (QFile::exists(arrLogFiles[i])) {
            QFile::rename(arrLogFiles[i], arrLogFiles[i-1]);
        }
    }

    if (fLogfile.isOpen()) {
        fLogfile.close();
    }

    fLogfile.setFileName(strLogfile);
    if (!fLogfile.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        cerr << "Failed to open log file " << strLogfile.toStdString() << endl;
    }

    if (NULL == pOldHandler) {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        pOldHandler = qInstallMessageHandler(myQt5MessageOutput);
#else
        pOldHandler = qInstallMsgHandler(myMessageOutput);
#endif
    }
}//initLogging

static void
deinitLogging()
{
    fLogfile.close();
}//deinitLogging

int
main(int argc, char **argv)
{
    initLogging();

    QCoreApplication app(argc, argv);
    MainWindow w;

    int rv = app.exec();

    deinitLogging();
    return (rv);
}//main
