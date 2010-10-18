#include "global.h"
#include "ObserverFactory.h"

#if defined (Q_OS_UNIX) && !defined (Q_OS_SYMBIAN)
#include "TpObserver.h"
#include <TelepathyQt4/ClientRegistrar>

ClientRegistrarPtr  clientRegistrar;
#endif

#if LINUX_DESKTOP || defined(Q_WS_WIN32)
#include "SkypeObserver.h"
#endif

ObserverFactory::ObserverFactory(QObject *parent)
: QObject(parent)
{
}//ObserverFactory::ObserverFactory

ObserverFactory::~ObserverFactory ()
{
#if defined (Q_OS_UNIX) && !defined (Q_OS_SYMBIAN)
    if (!clientRegistrar.isNull ())
    {
        // Don't know why deleting this object causes a segfault
        // ClientRegistrar *data = clientRegistrar.data ();
        // delete data;

        clientRegistrar.reset ();
    }
#endif
}//ObserverFactory::~ObserverFactory

bool
ObserverFactory::init ()
{
    // Observer for Telepathy on desktop Linux and Maemo 5
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
    TpObserver *myobserver = new TpObserver(filters, this);
    AbstractClientPtr appr = (AbstractClientPtr) myobserver;
    clientRegistrar->registerClient(appr, "QGVStreamObserver");

    QObject::connect (
            myobserver, SIGNAL (status(const QString &, int)),
            this      , SIGNAL (status(const QString &, int)));

    listObservers += (IObserver*) myobserver;
#endif

    // Observer for Skype on desktop Linux and desktop Windows
#if LINUX_DESKTOP || defined(Q_WS_WIN32)
    SkypeObserver *skypeObs = new SkypeObserver ();

    QObject::connect (
        skypeObs, SIGNAL (log(const QString &, int)),
        this    , SIGNAL (log(const QString &, int)));
    QObject::connect (
        skypeObs, SIGNAL (status(const QString &, int)),
        this    , SIGNAL (status(const QString &, int)));

    listObservers += (IObserver*) skypeObs;
#endif

    return (true);
}//ObserverFactory::init

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
