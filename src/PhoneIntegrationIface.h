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

#ifndef __PHONEINTEGRATIONIFACE_H__
#define __PHONEINTEGRATIONIFACE_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class IPhoneIntegration : public QObject
{
    Q_OBJECT

public:
    IPhoneIntegration(QObject *parent = NULL)
        : QObject (parent)
        , m_integrationEnabled(false) {}
    virtual bool isEnabled() { return m_integrationEnabled; }

Q_SIGNALS:
    void enableChanged(bool enabled);

public Q_SLOTS:
    virtual void phoneIntegrationChanged(bool enable = false) {
        Q_DEBUG(QString("User requested that phone integration be %1")
                .arg (enable ? "enabled" : "disabled"));
        m_integrationEnabled = enable;
    }

protected:
    bool m_integrationEnabled;
};

#endif //__PHONEINTEGRATIONIFACE_H__
