#ifndef SKYPECLIENTFACTORY_H
#define SKYPECLIENTFACTORY_H

#include "global.h"

class SkypeClient;
typedef QMap<QString, SkypeClient *> SkypeClientMap;

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class SkypeClientFactory : public QObject
{
    Q_OBJECT

public:
    void setMainWidget (QWidget *win);

    SkypeClient *ensureSkypeClient (const QString &name);
    bool deleteClient (const QString &name);

private:
    explicit SkypeClientFactory(QObject *parent = 0);
    ~SkypeClientFactory ();

signals:
    void status (const QString &txt, int timeout = 0);

private:
    SkypeClientMap mapClients;

    QWidget *mainwin;

    friend class Singletons;
};

#endif // SKYPECLIENTFACTORY_H
