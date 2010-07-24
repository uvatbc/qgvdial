#include "CallInitiatorFactory.h"

#if (defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)) || defined (Q_WS_WIN32)
#include "DesktopSkypeCallInitiator.h"
#endif

#if defined(Q_WS_X11)
#include "TpCalloutInitiator.h"
#endif

CallInitiatorFactory::CallInitiatorFactory (QObject *parent)
: QObject(parent)
{
    init ();
}//CallInitiatorFactory::CallInitiatorFactory

const CalloutInitiatorList &
CallInitiatorFactory::getInitiators ()
{
    return (listInitiators);
}//CallInitiatorFactory::getInitiators

void
CallInitiatorFactory::init ()
{
    CalloutInitiator *initiator = NULL;
#if (defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)) || defined (Q_WS_WIN32)
    initiator = new DesktopSkypeCallInitiator (this);
    listInitiators += initiator;
#endif

#if defined(Q_WS_X11)
    initiator = new TpCalloutInitiator (this);
    listInitiators += initiator;
#endif
}//CallInitiatorFactory::init
