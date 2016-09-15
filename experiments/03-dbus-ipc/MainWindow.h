#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "global.h"
#include "CtrlService.h"
#include "qgvn_adapter.h"
#include "qgvn_proxy.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

public slots:
    void log (const QString &strText, int level = 10);

private slots:
    void onCleanupLogsArray();
    void init();
    void onCommandForClient(const QString &command);

    void onTimerTick();

private:
    QString baseDir();
    void initLogging ();
    void closeEvent(QCloseEvent *event);

private:
    //! Mutex to protect access to all logs related variables
    QMutex          logMutex;
    //! This holds a circular buffer of log messages that will be shown by QML
    QStringList     arrLogMsgs;
    //! Logs display timer
    QTimer          logsTimer;
    //! kick timer
    bool            bKickLocksTimer;

    //! Object for DBus service
    CtrlService    *ctrlService;
    //! Object for DBus client
    QGVNotifyProxyIface *client;
};

#endif //_MAINWINDOW_H_
