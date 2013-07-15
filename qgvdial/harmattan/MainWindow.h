#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include "IMainWindow.h"
#include "qmlapplicationviewer.h"

class MainWindow : public IMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QmlApplicationViewer *parent = 0);
    void init();

signals:

public slots:

private:
    QmlApplicationViewer *m_view;
};

#endif // MAINWINDOW_H
