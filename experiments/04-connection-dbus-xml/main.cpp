#include "QGVConnectionManager.h"
#include <iostream>
using namespace std;

QtMsgHandler    pOldHandler = NULL;
QFile fLogfile;       //! Logfile
int   logLevel = 5;   //! Log level
int   logCounter = 0; //! Number of log entries since the last log flush
#define LOG_FLUSH_LEVEL 0 // 50

QStringList arrLogFiles;

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
    }

    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(msg);

    // Send to standard output.
    // I'm not using endl here because endl causes flushes
    cout << strLog.toAscii().constData() << "\n";

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
    QString strLogfile;
    /*
    OsDependent &osd = Singletons::getRef().getOSD ();
    strLogfile = osd.getAppDirectory ();
    */

    strLogfile = QDir::homePath() + "/.qgvdial/qgv-tp.log";

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
    fLogfile.close ();
}//deinitLogging

int
main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    initLogging();

    registerDBusTypes();

    QGVConnectionManager *cm = new QGVConnectionManager(&a);
    if (NULL == cm) {
        Q_WARN("Failed to allocate connection manager");
        return -1;
    }

    if (!cm->registerObject ()) {
        delete cm;
        Q_WARN("Failed to register connection manager");
        return -1;
    }
    Q_DEBUG("CM object and service registered. Start event loop");

    int rv = a.exec();

    deinitLogging();

    return rv;
}
