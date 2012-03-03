#include "MainWindow.h"
#include <iostream>
using namespace std;

QtMsgHandler pOldHandler = NULL;
MainWindow *pw = NULL;

QStringList arrLogFiles;
QFile fLogfile;       //! Logfile
int   logCounter = 0; //! Number of log entries since the last log flush

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

    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg);

    // Send to standard output
    cout << strLog.toStdString () << endl;

    if (NULL != pw) {
        pw->log (strMsg, level);
    }

    strLog += '\n';
    if (fLogfile.isOpen ()) {
        // Append it to the file
        fLogfile.write(strLog.toLatin1 ());

        ++logCounter;
        if (logCounter > 50) {
            qgv_LogFlush ();
        }
    }

    if (NULL == pOldHandler) {
        if (QtFatalMsg == type) {
            abort();
        }
    } else {
        pOldHandler (type, strMsg.toLatin1 ());
    }
}//myMessageOutput

QString
baseDir()
{
    QString strBasedir = QDir::homePath();
    QDir baseDir(strBasedir);
    if (!baseDir.exists (".qgvdial")) {
        baseDir.mkdir (".qgvdial");
    }
    strBasedir += QDir::separator();
    strBasedir += ".qgvdial";
    return strBasedir;
}//baseDir

static void
initLogging ()
{
    QString strLogfile = baseDir ();
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
main (int argc, char **argv)
{
    initLogging ();

    QCoreApplication app(argc, argv);
    MainWindow w;
    pw = &w;

    int rv = app.exec ();

    deinitLogging ();
    return (rv);
}//main
