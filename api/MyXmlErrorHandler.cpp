/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#include "MyXmlErrorHandler.h"

MyXmlErrorHandler::MyXmlErrorHandler(QObject *parent)
: QAbstractMessageHandler(parent)
{
}//MyXmlErrorHandler::MyXmlErrorHandler

void
MyXmlErrorHandler::handleMessage (QtMsgType type, const QString &description,
                                  const QUrl & /*identifier*/,
                                  const QSourceLocation &sourceLocation)
{
    QString msg = QString("XML message: %1, at uri= %2 "
                          "line %3 column %4")
                .arg(description)
                .arg(sourceLocation.uri ().toString ())
                .arg(sourceLocation.line ())
                .arg(sourceLocation.column ());

    switch (type)
    {
    case QtDebugMsg:
        qDebug() << msg;
        break;
    case QtWarningMsg:
        qWarning() << msg;
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        qCritical() << msg;
        break;
    }
}//MyXmlErrorHandler::handleMessage
