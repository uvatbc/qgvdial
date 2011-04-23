#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "global.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

public slots:
    void log (const QString &strText, int level = 10);

private:
    void initLogging ();

private:
    //! Logfile
    QFile           fLogfile;
};

#endif //_MAINWINDOW_H_
