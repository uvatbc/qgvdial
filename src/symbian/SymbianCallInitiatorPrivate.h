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

#ifndef SYMBIANCALLINITIATORPRIVATE_H
#define SYMBIANCALLINITIATORPRIVATE_H

#include <QtCore>
#include <Etel3rdParty.h>

class SymbianCallInitiator;
class SymbianCallInitiatorPrivate : public CActive
{
public:
    static SymbianCallInitiatorPrivate* NewL(SymbianCallInitiator *parent,
                                             const QString &strNumber);
    static SymbianCallInitiatorPrivate* NewLC(SymbianCallInitiator *parent,
                                              const QString &strNumber);
    ~SymbianCallInitiatorPrivate ();

    void DoCancel();

protected:
    void ConstructL(const TDesC& aNumber);

private:
    void RunL();
    SymbianCallInitiatorPrivate(SymbianCallInitiator *p);

private:
    CTelephony::TCallId           iCallId;
    CTelephony::TCallParamsV1     iCallParams;
    CTelephony::TCallParamsV1Pckg iCallParamsPckg;

    SymbianCallInitiator *parent;

    bool bUsable;
};

#endif // SYMBIANCALLINITIATORPRIVATE_H
