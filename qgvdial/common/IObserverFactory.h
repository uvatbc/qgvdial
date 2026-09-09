/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2025 Yuvraaj Kelkar

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

#ifndef IOBSERVERFACTORY_H
#define IOBSERVERFACTORY_H

#include <QObject>
#include "global.h"
#include "IObserver.h"

class IObserverFactory : public QObject
{
    Q_OBJECT

public:
    explicit IObserverFactory(QObject *parent = 0);
    virtual ~IObserverFactory();

    template <typename Receiver, typename Func>
    void startObservers (const QString &strContact,
                               Receiver *receiver  ,
                               Func      method    )
    {
        bool rv;
        foreach (IObserver *observer, listObservers) {
            rv = connect (observer, &IObserver::callStarted,
                          receiver, method);
            if (!rv) {
                Q_WARN(QString("Failed to connect observer \"%1\"")
                        .arg(observer->name()));
            }
            observer->startMonitoring (strContact);
        }
    }
    void stopObservers ();

signals:
    void status(const QString &strText, int timeout = 2000);

public slots:

private:
    IObserverList listObservers;
};

IObserverFactory *
createObserverFactory(QObject *parent);

#endif // IOBSERVERFACTORY_H
