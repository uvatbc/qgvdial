/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2016  Yuvraaj Kelkar

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

#ifndef LIB_H
#define LIB_H

#include "global.h"
#include "IOsDependent.h"

class Lib : public QObject
{
    Q_OBJECT
private:
    explicit Lib(QObject *parent = 0);

public:
    static Lib &ref();
    static void deref();

    inline QString getTempDir()   { return m_osd->getTempDir(); }
    inline QString getDbDir()     { return m_osd->getDbDir(); }
    inline QString getLogsDir()   { return m_osd->getLogsDir(); }
    inline QString getOsDetails() { return m_osd->getOsDetails(); }
    inline QString getVmailDir()  { return m_osd->getVmailDir(); }

    inline bool cipher(const QByteArray &byIn, QByteArray &byOut, bool bEncrypt) {
        return m_osd->cipher(byIn, byOut, bEncrypt);
    }

    // BE VERY CAREFUL USING THIS FUNCTION
    inline IOsDependant *osd() { return m_osd; }


protected:
    IOsDependant *m_osd;
};

#endif // LIB_H
