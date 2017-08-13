#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "global.h"

class CQmlViewer;

class MainWindow : public QObject
{
    Q_OBJECT

public:
    MainWindow(QObject *parent = NULL);
    ~MainWindow();

private slots:
    void init();

private:
    QObject * getQMLObject(const char *pageName);

private:
    CQmlViewer *m_view;
};

#endif //_MAINWINDOW_H_
