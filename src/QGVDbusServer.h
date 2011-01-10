#ifndef QGVDBUSSERVER_H
#define QGVDBUSSERVER_H

#include "global.h"
#include <QtDBus>

class QGVDbusServerHelper : public QObject
{
    Q_OBJECT

public:
    explicit QGVDbusServerHelper (QObject *parent = 0);
    void emitDialNow (const QString &strNumber);
    void emitText (const QStringList &arrNumbers,
                   const QString     &strData);
    void emitTextWithoutData (const QStringList &arrNumbers);

signals:
    void dialNow (const QString &strNumber);
    void sendText (const QStringList &arrNumbers,
                   const QString     &strData);
    void sendTextWithoutData (const QStringList &arrNumbers);
};

class QGVDbusCallServer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.QGVDial.CallServer")

protected:
    explicit QGVDbusCallServer(QObject *parent = 0);

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

class QGVDbusTextServer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.QGVDial.TextServer")

protected:
    explicit QGVDbusTextServer(QObject *parent = 0);

    void addTextReceivers (QObject *r1, const char *m1,
                           QObject *r2, const char *m2);
    void delTextReceivers (QObject *r1, const char *m1,
                           QObject *r2, const char *m2);

public slots:
    Q_NOREPLY void Text (const QStringList &arrNumbers,
                         const QString     &strData);
    Q_NOREPLY void TextWithoutData (const QStringList &arrNumbers);

signals:

public slots:

protected:
    QGVDbusServerHelper helper;

    friend class OsDependent;
};

#include "OsDependent.h"

#endif // QGVDBUSSERVER_H
