#ifndef CONSOLETHREAD_H
#define CONSOLETHREAD_H

#include <QtCore>

class ConsoleThread : public QThread
{
    Q_OBJECT
public:
    explicit ConsoleThread(QObject *parent = 0);
    void run();

signals:

public slots:

};

#endif // CONSOLETHREAD_H
