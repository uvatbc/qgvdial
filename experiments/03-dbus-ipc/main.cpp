#include "MainWindow.h"

#include <iostream>
using namespace std;

QtMsgHandler pOldHandler = NULL;
MainWindow *pw = NULL;
QStringList arrLogFiles;
QFile       fLogfile;       //! Logfile
int         logCounter = 0; //! Number of log entries since the last log flush

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

    QRegExp regex("^\"(.*)\"\\s*");
    if (strMsg.indexOf (regex) != -1) {
        strMsg = regex.cap (1);
    }

    QDateTime dt = QDateTime::currentDateTime ();
    strMsg = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(strMsg);

    if (fLogfile.isOpen ()) {
        // Append it to the file
        fLogfile.write(strMsg.toLatin1());

        ++logCounter;
        if (logCounter > 100) {
            logCounter = 0;
            fLogfile.flush ();
        }
    }

    if (NULL != pw) {
        pw->log (strMsg, level);
    }
    cout << strMsg.toStdString() << endl;

    if (NULL == pOldHandler) {
        if (NULL == pw) {
            strMsg += "\n";
            fwrite (strMsg.toLatin1 (), strMsg.size (), 1, stderr);
        }

        if (QtFatalMsg == type) {
            abort();
        }
    }
}//myMessageOutput

static void
initLogging ()
{
    QString strBasedir = QDir::homePath();
    QDir baseDir(strBasedir);
    if (!baseDir.exists (".logs")) {
        baseDir.mkdir (".logs");
    }
    strBasedir += QDir::separator();
    strBasedir += ".logs";

    QString strLogfile = strBasedir;
    strLogfile += QDir::separator ();
    strLogfile += "03_dbus.log";

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
main (int argc, char **argv)
{
    initLogging ();

    QApplication app(argc, argv);
    MainWindow w;
    pw = &w;

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    int rv = app.exec ();
    deinitLogging ();
    return rv;
}//main
