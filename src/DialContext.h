#ifndef __DIALCONTEXT_H__
#define __DIALCONTEXT_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class CalloutInitiator;
class DialCancelDlg;

class DialContext : public QObject
{
public:
    DialContext(QObject *parent);
    ~DialContext();

    bool bDialOut;
    CalloutInitiator *ci;
    DialCancelDlg *pDialDlg;
};

#endif //__DIALCONTEXT_H__
