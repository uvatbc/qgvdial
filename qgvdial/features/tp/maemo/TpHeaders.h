/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#ifndef __TPHEADERS_H__
#define __TPHEADERS_H__

#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/PendingChannelRequest>
#include <TelepathyQt4/PendingAccount>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/SharedPtr>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/AbstractClientObserver>
#include <TelepathyQt4/ChannelDispatchOperation>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ChannelRequest>
#include <TelepathyQt4/ChannelClassSpecList>
#include <TelepathyQt4/Profile>

#ifdef DBG_TP_VERBOSE
#include <TelepathyQt4/Debug>
#endif

#define TPQT_CHANNEL_TYPE_STREAMED_MEDIA TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA
#define TPQT_IFACE_CHANNEL TELEPATHY_INTERFACE_CHANNEL

using namespace Tp;

#endif//__TPHEADERS_H__
