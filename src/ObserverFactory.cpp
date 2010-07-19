#include "ObserverFactory.h"

#if defined (Q_OS_UNIX) && !defined (Q_OS_SYMBIAN)
#include "TpObserver.h"
#include <TelepathyQt4/ClientRegistrar>

ClientRegistrarPtr  clientRegistrar;
#endif

ObserverFactory::ObserverFactory(QObject *parent)
: QObject(parent)
{
#if defined (Q_OS_UNIX) && !defined (Q_OS_SYMBIAN)
    clientRegistrar = ClientRegistrar::create();

    ChannelClassList filters;
    QMap<QString, QDBusVariant> filter;
    filter.insert(
            QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
            QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA));
    filter.insert(
            QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
            QDBusVariant((uint) Tp::HandleTypeContact));
    filters.append(filter);
    TpObserver *myobserver = new TpObserver(filters);
    AbstractClientPtr appr = (AbstractClientPtr) myobserver;
    clientRegistrar->registerClient(appr, "QGVStreamObserver");

    QObject::connect (
            myobserver, SIGNAL (log(const QString &, int)),
            this      , SIGNAL (log(const QString &, int)));
    QObject::connect (
            myobserver, SIGNAL (status(const QString &, int)),
            this      , SIGNAL (status(const QString &, int)));

    listObservers += (IObserver*) myobserver;
#endif
}//ObserverFactory::ObserverFactory

void
ObserverFactory::startObservers (const QString &strContact,
                                       QObject *receiver  ,
                                 const char    *method    )
{
    foreach (IObserver *observer, listObservers)
    {
        QObject::connect (observer, SIGNAL (callStarted()),
                          receiver, method);
        observer->startMonitoring (strContact);
    }
}//ObserverFactory::createObservers

void
ObserverFactory::stopObservers ()
{
    foreach (IObserver *observer, listObservers)
    {
        observer->stopMonitoring ();
        observer->disconnect (SIGNAL (callStarted()));
    }
}//ObserverFactory::stopObservers