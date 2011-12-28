#include "AsyncTaskToken.h"

AsyncTaskToken::AsyncTaskToken(QObject *parent)
: QObject(parent)
, status(ATTS_SUCCESS)
, callerCtx(NULL)
, apiCtx(NULL)
{
}//AsyncTaskToken::AsyncTaskToken

void
AsyncTaskToken::emitCompleted()
{
   emit completed (this);
}//AsyncTaskToken::emitCompleted
