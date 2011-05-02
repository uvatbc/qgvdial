#ifndef SYMBIANDTMFPRIVATE_H
#define SYMBIANDTMFPRIVATE_H

#include <QtCore>
#include <Etel3rdParty.h>

class SymbianCallInitiator;
class SymbianDTMFPrivate : public CActive
{
public:
    SymbianDTMFPrivate (SymbianCallInitiator *p);
    ~SymbianDTMFPrivate ();

    void sendDTMF (const QString &strTones);

private:
    void RunL();
    void DoCancel();

private:
    SymbianCallInitiator *parent;
};

#endif // SYMBIANDTMFPRIVATE_H
