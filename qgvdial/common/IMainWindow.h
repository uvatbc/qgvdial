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

#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include <QObject>
#include "CacheDb.h"
#include "GVApi.h"

/*==============================================================================
 * IMainWindow: The class that drives the entire business logic of qgvdial.
 * Ever new platform MUST derive this class and have the actual UI class as a
 * private member (d_ptr pattern).
 * There are a bunch of pure virtual functions that MUST be implemented by the
 * derived class to complete the functionality on the UI side of things.
 *
 *==============================================================================
 * Initialization sequence:
 * Constructor -> init() -> onInitDone()
 * -> Constructor: You don't have to construct everything here.
 * -> init(): You should begin init here. Invoked before event loop. This
 *      function MUST initialize the UI, connect signals and either ensure that
 *      the onInitDone() function is either synchronosly called or scheduled for
 *      execution with a dummy single shot timer OR (as in the case of QML) be
 *      started when the UI has successfully finished loading.
 * -> onInitDone(): This is where I decide whether to automatically begin login
 *      if the credendtionas are saved or request the user for credentials if
 *      they are not saved.
 *
 *==============================================================================
 * Login credentials NOT saved:
 * onInitDone() -> uiRequestLoginDetails() -> beginLogin()
 *    [ -> uiRequestTFALoginDetails() -> resumeTFAAuth() ]
 *      -> uiLoginCompleted(...)
 *
 * -> onInitDone(): If the credentials are NOT saved in the cache, then I need
 *      to go ask the user for it. This varies based on the UI and thus must be
 *      implemented by the derived class (DC). As so:
 * -> uiRequestTFALoginDetails(): In this function, the DC must open the
 *      settings page and present the user with the dialog to enter the user
 *      name (email) and the password. On getting the email and password the
 *      next step is to invoke beginLogin()
 * -> beginLogin(): This will invoke the GV api class to begin the login. If the
 *      user has configured two-factor authentication, then I will invoke the
 *      uiRequestTFALoginDetails()
 * -> uiRequestTFALoginDetails(): DC must present the user with a dialog to get
 *      the two factor authentication pin. If the user provides the pin, DC must
 *      invoke resumeTFAAuth(ctx, pin, false)
 *      If the user is unable to provide the pin because he couldn't get the
 *      text message with the pin, then there is the option of asking Google
 *      Voice to provide the pin with an automated voice call. To do this, DC
 *      must call resumeTFAAuth(ctx, pin, true).
 * -> uiLoginCompleted(): At the end of the login process, the BC will invoke
 *      this function - with either a success status or an error with a
 *      non-empty error string.
 *
 *==============================================================================
 * Login credentials saved:
 * Usually: onInitDone() -> uiLoginCompleted(...)
 * Sometimes the api may invoke the TFA functions - depends on when Google
 * thinks it is necessary.
 *
 *==============================================================================
 */

class IMainWindow : public QObject
{
    Q_OBJECT
public:
    explicit IMainWindow(QObject *parent = 0);
    virtual void init() = 0;

protected slots:
    void onInitDone();
    void resumeTFAAuth(void *ctx, int pin, bool useAlt);
    void onQuit();

private slots:
    void onTFARequest(AsyncTaskToken *task);
    void loginCompleted(AsyncTaskToken *task);

protected:
    virtual void log(QDateTime dt, int level, const QString &strLog) = 0;

    virtual void uiRequestLoginDetails() = 0;
    virtual void uiRequestTFALoginDetails(void *ctx) = 0;
    virtual void uiSetUserPass(const QString &user, const QString &pass,
                               bool editable) = 0;

    void beginLogin(const QString &user, const QString &pass);
    virtual void uiLoginDone(int status, const QString &errStr) = 0;

protected:
    CacheDb db;
    GVApi   api;

    AsyncTaskToken *loginTask;
};

#endif // IMAINWINDOW_H
