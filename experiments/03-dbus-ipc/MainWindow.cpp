#include "MainWindow.h"

#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
: QMainWindow(parent, flags)
, logMutex (QMutex::Recursive)
, logsTimer (this)
, bKickLocksTimer (false)
, ctrlService (NULL)
, client (NULL)
{
    initLogging ();

    QTimer::singleShot(10, this, SLOT(init()));
}//MainWindow::MainWindow

QString
MainWindow::baseDir()
{
    QString strBasedir = QDir::homePath();
    QDir baseDir(strBasedir);
    if (!baseDir.exists (".logs")) {
        baseDir.mkdir (".logs");
    }
    strBasedir += QDir::separator();
    strBasedir += ".logs";
    return strBasedir;
}//MainWindow::baseDir

void
MainWindow::initLogging ()
{
    // Initialize logging
    logsTimer.setSingleShot (true);
    logsTimer.start (3 * 1000);
    bool rv = connect (&logsTimer, SIGNAL(timeout()),
                        this     , SLOT(onCleanupLogsArray()));
    Q_ASSERT(rv); Q_UNUSED(rv);
}//MainWindow::initLogging

void
MainWindow::onCleanupLogsArray()
{
    int timeout = 3 * 1000;
    do { // Begin cleanup block (not a loop)
        QMutexLocker locker(&logMutex);
        if (!bKickLocksTimer) {
            break;
        }
        bKickLocksTimer = false;
        timeout = 1 * 1000;

        //TODO: Add the code to append the log messages array to the logs window
        
        arrLogMsgs.clear ();
    } while (0); // End cleanup block (not a loop)

    logsTimer.start (timeout);
}//MainWindow::onCleanupLogsArray

/** Log information to console and to log file
 * This function is invoked from the qDebug handler that is installed in main.
 * @param strText Text to be logged
 * @param level Log level
 */
void
MainWindow::log (const QString &strText, int /*level = 10*/)
{
    // Append it to the circular buffer
    QMutexLocker locker(&logMutex);
    arrLogMsgs.append (strText);
    bKickLocksTimer = true;
}//MainWindow::log

void
MainWindow::init()
{
    int option = 0;
    QStringList args = qApp->arguments();

    if (args.count() == 2) {
        if (args[1] == "c") {
            Q_DEBUG("Client");
            option = 1;
        } else if (args[1] == "s") {
            Q_DEBUG("Server");
            option = 2;
        }
    }

    if (option == 1) {          // Client
        client = new QGVNotifyProxyIface("net.yuvraaj.qgvnotify.control", "/",
                                         QDBusConnection::sessionBus(), 0);
        connect(client, SIGNAL(CommandForClient(const QString &)),
                this  , SLOT(onCommandForClient(const QString &)));
    } else if (option == 2) {   // Server
        ctrlService = new CtrlService(this);
        new QGVNotifyIfaceAdapter(ctrlService);

        QDBusConnection connection = QDBusConnection::sessionBus();
        bool ret = connection.registerService("net.yuvraaj.qgvnotify.control");
        ret = connection.registerObject("/", ctrlService);

        QTimer::singleShot(5 * 1000, this, SLOT(onTimerTick()));
    } else {
        Q_DEBUG("Invalid parameters list");
        return;
    }
}//MainWindow::init

void
MainWindow::onCommandForClient(const QString &command)
{
    Q_DEBUG("command = ") << command;

    if (NULL == client) {
        Q_WARN("client is NULL");
        return;
    }

    if (command == "getUser") {
        client->ReportUser("me@gmail.com", "/tmp/settings", "/tmp/logs",
                           qApp->applicationPid());
    } else if (command == "quitAll") {
        close();
    }
}//MainWindow::onCommandForClient

void
MainWindow::onTimerTick()
{
    if (NULL == ctrlService) {
        Q_WARN("Control service not allocated");
        return;
    }

    ctrlService->requestUserInfo();

    QTimer::singleShot(5 * 1000, this, SLOT(onTimerTick()));
}//MainWindow::onTimerTick

void
MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    if (NULL != ctrlService) {
        ctrlService->requestAllQuit();
    }
}//MainWindow::closeEvent

