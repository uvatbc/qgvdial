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

protected:
    void ConstructL(const TDesC& aNumber);

private:
    void RunL();
    void DoCancel();
    SymbianCallInitiatorPrivate(SymbianCallInitiator *p);

private:
    CTelephony::TCallId           iCallId;
    CTelephony::TCallParamsV1     iCallParams;
    CTelephony::TCallParamsV1Pckg iCallParamsPckg;

    SymbianCallInitiator *parent;

    bool bUsable;
};

#endif // SYMBIANCALLINITIATORPRIVATE_H
