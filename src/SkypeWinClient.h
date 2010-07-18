#ifndef SKYPEWINCLIENT_H
#define SKYPEWINCLIENT_H

#include <QtGui>
#include "SkypeClientFactory.h"
#include "SkypeClient.h"

class SkypeWinClient : public SkypeClient
{
    Q_OBJECT

private:
    SkypeWinClient(const QWidget &mainwidget,
                   const QString &name      ,
                         QObject *parent = 0);

private slots:
    void skypeAttachStatus (bool bOk);
    void skypeNotify (const QString &strData);

private:
    bool ensureConnected ();
    bool invoke (const QString &strCommand);

private:
    const QWidget &mainwin;
    bool  bInvokeInProgress;

    friend class SkypeClientFactory;
};

#endif // SKYPEWINCLIENT_H
