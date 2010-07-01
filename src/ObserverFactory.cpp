#include "ObserverFactory.h"

#if defined (Q_OS_UNIX)
#include "TpObserver.h"
#include <TelepathyQt4/ClientRegistrar>

ClientRegistrarPtr  clientRegistrar;
#endif

ObserverFactory::ObserverFactory(QObject *parent)
: QObject(parent)
{
#if defined (Q_OS_UNIX)
    clientRegistrar = ClientRegistrar::create();
#endif
}//ObserverFactory::ObserverFactory

ObserverFactory &
ObserverFactory::getRef ()
{
    static ObserverFactory singleton;
    return (singleton);
}//ObserverFactory::getRef

IObserverList
ObserverFactory::createObservers (const QString &strContact)
{
    IObserverList list;

#if defined (Q_OS_UNIX)
    ChannelClassList filters;
    QMap<QString, QDBusVariant> filter;
    filter.insert(
            QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"),
            QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA));
    filter.insert(
            QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"),
            QDBusVariant((uint) Tp::HandleTypeContact));
    filters.append(filter);
    TpObserver *myobserver = new TpObserver(filters, strContact);
    AbstractClientPtr appr = (AbstractClientPtr) myobserver;
    clientRegistrar->registerClient(appr, "QGVStreamObserver");

    QObject::connect (
            myobserver, SIGNAL (log(const QString &, int)),
            this      , SIGNAL (log(const QString &, int)));
    QObject::connect (
            myobserver, SIGNAL (status(const QString &, int)),
            this      , SIGNAL (status(const QString &, int)));

    list += (IObserver*) myobserver;
#endif
    return (list);
}//ObserverFactory::createObservers
