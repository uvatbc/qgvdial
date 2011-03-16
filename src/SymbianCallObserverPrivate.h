#ifndef SYMBIANCALLOBSERVERPRIVATE_H
#define SYMBIANCALLOBSERVERPRIVATE_H

#include <QtCore>
#include <Etel3rdParty.h>

class SymbianCallInitiator;
class SymbianCallObserverPrivate : public CActive
{
public:
    static SymbianCallObserverPrivate* NewL(SymbianCallInitiator *parent);
    static SymbianCallObserverPrivate* NewLC(SymbianCallInitiator *parent);
    ~SymbianCallObserverPrivate ();

private:
    SymbianCallObserverPrivate(SymbianCallInitiator *p);
    void RunL();
    void DoCancel();
    void ConstructL();
    void StartListening ();

private:
    CTelephony*                   iTelephony;
    TInt                          iState;
    CTelephony::TCallStatusV1     iCurrentStatus;
    CTelephony::TCallStatusV1Pckg iCurrentStatusPckg;

    SymbianCallInitiator *parent;

    bool bUsable;
};

#endif // SYMBIANCALLOBSERVERPRIVATE_H
