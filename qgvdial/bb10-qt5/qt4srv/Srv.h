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

#ifndef _SRV_H_INCLUDED_
#define _SRV_H_INCLUDED_

#include <QtCore>
#include <QtNetwork>

#ifndef Q_WS_SIMULATOR
#include <bb/system/phone/Phone>
#include <bb/system/phone/Line>
#include <bb/system/phone/LineType>
#endif

class MainObject : public QObject
{
   Q_OBJECT

public:
   MainObject(QObject *parent = NULL);
   ~MainObject();

private slots:
   void onNewConnection();
   void saveMessage(const QString &msg);
   void onStateChanged(QLocalSocket::LocalSocketState socketState);
   void onReadyRead();

   void onCallCommandResponseReceived(const bb::system::phone::CallCommandResponse &commandResponse);

private:
   QLocalServer m_srv;
   QStringList m_msgs;
   QLocalSocket *m_single;
   
   bool m_wakeupFirst;

#ifndef Q_WS_SIMULATOR
   bb::system::phone::Phone m_phone;
#endif
};

#endif//_SRV_H_INCLUDED_

