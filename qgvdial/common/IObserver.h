/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2015  Yuvraaj Kelkar

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

#ifndef IOBSERVER_H
#define IOBSERVER_H

#include <QObject>
#include <QtCore>

class IObserver : public QObject
{
    Q_OBJECT

public:
    virtual QString name() = 0;

protected:
    IObserver (QObject *parent = NULL) : QObject(parent) {}

signals:
    void status(const QString &strText, int timeout = 2000);

    void callStarted ();

protected:
    virtual void startMonitoring (const QString &strC) = 0;
    virtual void stopMonitoring () = 0;

protected:
    QString strContact;

    friend class IObserverFactory;
};
typedef QList<IObserver *> IObserverList;

#endif // IOBSERVER_H
