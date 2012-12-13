/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#ifndef SKYPECLIENTFACTORY_H
#define SKYPECLIENTFACTORY_H

#include "global.h"

class SkypeClient;
typedef QMap<QString, SkypeClient *> SkypeClientMap;

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class SkypeClientFactory : public QObject
{
    Q_OBJECT

public:
    void setMainWidget (QWidget *win);

    SkypeClient *ensureSkypeClient (const QString &name);
    bool deleteClient (const QString &name);

private:
    explicit SkypeClientFactory(QObject *parent = 0);
    ~SkypeClientFactory ();

signals:
    void status (const QString &txt, int timeout = 0);

private:
    SkypeClientMap mapClients;

    QWidget *mainwin;

    friend class Singletons;
};

#endif // SKYPECLIENTFACTORY_H
