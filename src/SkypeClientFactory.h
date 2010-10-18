#ifndef SKYPECLIENTFACTORY_H
#define SKYPECLIENTFACTORY_H

#include <QtCore>

class SkypeClient;
typedef QMap<QString, SkypeClient *> SkypeClientMap;

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
