#include "DialContext.h"
#include "Singletons.h"
#include "DialCancelDlg.h"

DialContext::DialContext (QObject *parent)
: QObject(parent)
, bDialOut (false)
, ci (NULL)
, pDialDlg (NULL)
{
}//DialContext::DialContext

DialContext::~DialContext() {
    if (NULL != pDialDlg) {
        pDialDlg->deleteLater ();
        pDialDlg = NULL;
    }
}//DialContext::~DialContext
