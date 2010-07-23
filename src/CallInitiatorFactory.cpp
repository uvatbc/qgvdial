#include "CallInitiatorFactory.h"

#if (defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)) || defined (Q_WS_WIN32)
#include "DesktopSkypeCallInitiator.h"
#endif

CallInitiatorFactory::CallInitiatorFactory (QObject *parent)
: QObject(parent)
{
}//CallInitiatorFactory::CallInitiatorFactory

const CalloutInitiatorList &
CallInitiatorFactory::getInitiators ()
{
    return (listInitiators);
}//CallInitiatorFactory::getInitiators

void
CallInitiatorFactory::init ()
{
#if (defined(Q_WS_X11) && !defined(Q_WS_MAEMO_5)) || defined (Q_WS_WIN32)
    CalloutInitiator *initiator = new DesktopSkypeCallInitiator (this);
    listInitiators += initiator;
#endif
}//CallInitiatorFactory::init
