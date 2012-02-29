#ifndef ASYNCTASKTOKEN_H
#define ASYNCTASKTOKEN_H

#include "global.h"
#include <QObject>

#define ATTS_SUCCESS            0
#define ATTS_FAILURE            1
#define ATTS_INVALID_PARAMS     2
#define ATTS_LOGIN_FAILURE      3
#define ATTS_AC_NOT_CONFIGURED  4
#define ATTS_NOT_LOGGED_IN      5

class AsyncTaskToken : public QObject
{
    Q_OBJECT
public:
    explicit AsyncTaskToken(QObject *parent = 0);
    void emitCompleted();

signals:
    void completed(AsyncTaskToken *self);
    void cancel(AsyncTaskToken *self);

public slots:

public:
    QVariantMap inParams;
    QVariantMap outParams;

    int         status;

    // Context to be used by the caller
    void        *callerCtx;

    // context to be used by the API to which this token is passed.
    void        *apiCtx;

    //! If the work completed in error, then the error string is stored here.
    QString     errorString;
};

#endif // ASYNCTASKTOKEN_H
