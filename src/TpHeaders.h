#ifndef __TPHEADERS_H__
#define __TPHEADERS_H__

#include "global.h"

#if TELEPATHY_CAPABLE

#ifndef TP10
#error
#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>
#ifdef DBG_TP_VERBOSE
#include <TelepathyQt/Debug>
#endif
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/Connection>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/SharedPtr>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Channel>
#include <TelepathyQt/StreamedMediaChannel>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/ChannelDispatchOperation>
#include <TelepathyQt/StreamedMediaChannel>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Account>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ChannelRequest>
#include <TelepathyQt/ChannelClassSpecList>
#else
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>
#ifdef DBG_TP_VERBOSE
#include <TelepathyQt4/Debug>
#endif
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/SharedPtr>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/AbstractClientObserver>
#include <TelepathyQt4/ChannelDispatchOperation>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ChannelRequest>
#include <TelepathyQt4/ChannelClassSpecList>
#endif

using namespace Tp;

#endif//TELEPATHY_CAPABLE

#endif//__TPHEADERS_H__
