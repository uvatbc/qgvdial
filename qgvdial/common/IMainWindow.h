#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include <QObject>

class IMainWindow : public QObject
{
    Q_OBJECT
public:
    explicit IMainWindow(QObject *parent = 0);
    virtual void init() = 0;

signals:

public slots:

};

#endif // IMAINWINDOW_H
