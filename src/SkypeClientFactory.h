#ifndef SKYPECLIENTFACTORY_H
#define SKYPECLIENTFACTORY_H

#include <QObject>

class SkypeClient;

class SkypeClientFactory : public QObject
{
    Q_OBJECT

public:
    SkypeClient *createSkypeClient (QWidget &mainwin, const QString &name);

private:
    explicit SkypeClientFactory(QObject *parent = 0);

signals:
    void log (const QString &txt, int level = 10);
    void status (const QString &txt, int timeout = 0);

    friend class Singletons;
};

#endif // SKYPECLIENTFACTORY_H
