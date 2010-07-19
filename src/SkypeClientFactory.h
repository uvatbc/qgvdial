#ifndef SKYPECLIENTFACTORY_H
#define SKYPECLIENTFACTORY_H

#include <QtCore>

class SkypeClient;
typedef QList<SkypeClient *> SkypeClientList;

class SkypeClientFactory : public QObject
{
    Q_OBJECT

public:
    SkypeClient *createSkypeClient (QWidget &mainwin, const QString &name);
    bool deleteClient (SkypeClient *skypeClient);

private:
    explicit SkypeClientFactory(QObject *parent = 0);
    ~SkypeClientFactory ();

signals:
    void log (const QString &txt, int level = 10);
    void status (const QString &txt, int timeout = 0);

private:
    SkypeClientList listClients;

    friend class Singletons;
};

#endif // SKYPECLIENTFACTORY_H
