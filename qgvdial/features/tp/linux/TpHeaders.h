/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>
#include <TelepathyQt/PendingChannelRequest>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/Connection>
#include <TelepathyQt/SharedPtr>
#include <TelepathyQt/Channel>
#include <TelepathyQt/StreamedMediaChannel>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClientObserver>
#include <TelepathyQt/ChannelDispatchOperation>
#include <TelepathyQt/StreamedMediaChannel>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Account>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ChannelRequest>
#include <TelepathyQt/ChannelClassSpecList>
#include <TelepathyQt/Profile>

#ifdef DBG_TP_VERBOSE
#include <TelepathyQt/Debug>
#endif

#define TPQT_CHANNEL_TYPE_STREAMED_MEDIA TP_QT_IFACE_CHANNEL_TYPE_STREAMED_MEDIA
#define TPQT_IFACE_CHANNEL TP_QT_IFACE_CHANNEL

using namespace Tp;

#endif//__TPHEADERS_H__
