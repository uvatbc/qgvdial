#ifndef QGVDBUSSERVER_H
#define QGVDBUSSERVER_H

#include <QtDBus>

class QGVDbusServerHelper : public QObject
{
    Q_OBJECT

public:
    explicit QGVDbusServerHelper (QObject *parent = 0);
    void emitDialNow (const QString &strNumber);

signals:
    void dialNow (const QString &strNumber);
};

class QGVDbusServer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.QGVDial.CallServer")

protected:
    explicit QGVDbusServer(QObject *parent = 0);

    void addCallReceiver (QObject *receiver, const char *method);
    void delCallReceiver (QObject *receiver, const char *method);

public slots:
    Q_NOREPLY void Call (const QString &strNumber);

signals:

public slots:

protected:
    QGVDbusServerHelper helper;

    friend class OsDependent;
};

#include "OsDependent.h"

#endif // QGVDBUSSERVER_H
