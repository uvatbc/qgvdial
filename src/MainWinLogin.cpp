/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

#include "MainWindow.h"


/** Invoked to begin the login process.
 * We already have the username and password, so just start the login to the GV
 * website. The async completion routine is loginCompleted.
 */
void
MainWindow::doLogin ()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    AsyncTaskToken *token = NULL;
    bool bOk = false;

    do { // Begin cleanup block (not a loop)
        strUser = strUser.simplified ();
        strUser = strUser.remove (QChar(' '));
        if (strUser.isEmpty ()) {
            Q_WARN("Username is empty!");
            setStatus (tr("Username is empty!"), 0);
            showMsgBox (tr("Username is empty!"));
            break;
        }
        if (!strUser.contains ("@")) {
            Q_WARN(QString("Username without domain! %1").arg (strUser));
            showMsgBox (tr("Please enter the full username, eg: user@gmail.com"));
            break;
        }

        token = new AsyncTaskToken(this);
        if (!token) {
            Q_WARN("Failed to allocate token");
            break;
        }

        bOk = connect(token, SIGNAL(completed(AsyncTaskToken*)),
                      this , SLOT(loginCompleted(AsyncTaskToken*)));

        token->inParams["user"] = strUser;
        token->inParams["pass"] = strPass;

        Q_DEBUG("Login using user ") << strUser;
        setStatus ("Logging in...", 0);

        osd.setLongWork (this, true);

        gvApiProgressString = "Login progress";
        if (!gvApi.login (token)) {
            Q_WARN("Login returned immediately with failure!");
            osd.setLongWork (this, false);
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk) {
        if (token) {
            token->deleteLater ();
            token = NULL;
        }

        // Cleanup if any
        strUser.clear ();
        strPass.clear ();

        logoutCompleted (NULL);
    }
}//MainWindow::doLogin

void
MainWindow::onUserTextChanged (const QString &strUsername)
{
    if (strUser != strUsername) {
        strUser = strUsername;
        this->setUsername (strUser);

        // Clear out cookies so that any existing login credentials are cleared
        // out of the cache.
        QList<QNetworkCookie> noCookies;
        gvApi.setAllCookies(noCookies);

        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.clearCookies ();
    }
}//MainWindow::onUserTextChanged

void
MainWindow::onPassTextChanged (const QString &strPassword)
{
    if (strPass != strPassword) {
        strPass = strPassword;
        this->setPassword (strPass);

        // Clear out cookies so that any existing login credentials are cleared
        // out of the cache. Do this even if it is just the password because
        // who knows what may have changes in the credentials? Why take chances?
        QList<QNetworkCookie> noCookies;
        gvApi.setAllCookies(noCookies);

        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.clearCookies ();
    }
}//MainWindow::onUserPassTextChanged

/** SLOT: Invoked when user triggers the login/logout action
 * If it is a login action, the Login dialog box is shown.
 */
void
MainWindow::on_action_Login_triggered ()
{
    if (!bLoggedIn) {
        QDeclarativeContext *ctx = this->rootContext();
        ctx->setContextProperty ("g_bShowLoginSettings", true);
    } else {
        doLogout ();
    }
}//MainWindow::on_action_Login_triggered

void
MainWindow::loginCompleted (AsyncTaskToken *token)
{
    gvApiProgressString.clear ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    if (!token) {
        Q_CRIT("NULL TOKEN!!");
        setStatus ("Severe internal error");
        this->showMsgBox ("Severe internal error");

        dbMain.clearUser ();
        dbMain.clearPass ();
        dbMain.clearContactsPass ();
        dbMain.setTFAFlag (false);

        QTimer::singleShot (500, this, SLOT(onRecreateCookieJar()));
        return;
    }

    if (token->status != ATTS_SUCCESS) {
        logoutCompleted (NULL);

        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setLongWork (this, false);

        setStatus ("User login failed", 30*1000);

        QString strErr;
        if (token) {
            strErr = token->errorString;
        }
        if (strErr.isEmpty ()) {
            strErr = "User login failed";
        }
        this->showMsgBox (strErr);

        Q_WARN(QString("User login failed. Error string: %1").arg(strErr));

        if (token->status != ATTS_TIMEDOUT) {
            dbMain.clearUser ();
            dbMain.clearPass ();
            dbMain.clearContactsPass ();
            dbMain.setTFAFlag (false);
        }

        QTimer::singleShot (500, this, SLOT(onRecreateCookieJar()));

        if (token->status == ATTS_LOGIN_FAIL_SHOWURL) {
            QUrl url = token->outParams["nextUrl"].toUrl ();
            QDesktopServices::openUrl (url);
        }
    } else {
        setStatus ("User logged in");
        Q_DEBUG ("User logged in");

        QString strOldUser, strOldPass;
        dbMain.getUserPass (strOldUser, strOldPass);
        if (strOldUser != strUser) {
            // Cleanup cache
            dbMain.blowAwayCache ();
            dbMain.ensureCache ();
        }

        bool bInitContacts = true;
        QString contactsPass = strPass;
        if (dbMain.getTFAFlag ()) {
            if (!dbMain.getContactsPass (contactsPass)) {
                connect(this, SIGNAL(sigMessageBoxDone(bool)),
                        this, SLOT(onAppPassMsgBoxDone(bool)));

                QObject *obj = getQMLObject ("MsgBox");
                if (NULL != obj) {
                    obj->setProperty ("inputText", "");
                }
                Q_ASSERT(obj);

                showInputBox ("Enter app specific password for Google contacts");
                bInitContacts = false;
            }
        }

        if (bInitContacts) {
            // Prepare the contacts
            initContacts (contactsPass);
        }

        // Prepare the inbox widget for usage
        initInbox ();

        // Allow access to buttons and widgets
        actLogin.setText ("Logout");
        bLoggedIn = true;

        // Save the user name and password that was used to login
        dbMain.putUserPass (strUser, strPass);

        this->setUsername (strUser);
        this->setPassword (strPass);
        QDeclarativeContext *ctx = this->rootContext();
        ctx->setContextProperty ("g_bIsLoggedIn", bLoggedIn);
        bool bTemp = false;
        ctx->setContextProperty ("g_bShowLoginSettings", bTemp);

        // Fill up the combobox on the main page
        refreshRegisteredNumbers ();

        // Fill up the mq settings
        initMq ();

        // Fill up the pin settings
        refreshPinSettings ();
    }

    if (token) {
        delete token;
    }
}//MainWindow::loginCompleted

void
MainWindow::doLogout ()
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    connect (token, SIGNAL(completed(AsyncTaskToken*)),
             this, SLOT(logoutCompleted(AsyncTaskToken*)));

    if (!gvApi.logout (token)) {
        token->deleteLater ();
        return;
    }

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, true);

    oContacts->deinitModel ();
    oInbox->deinitModel ();

#if MOSQUITTO_CAPABLE
    mqThread.setQuit ();
#endif
}//MainWindow::doLogout

void
MainWindow::logoutCompleted (AsyncTaskToken *token)
{
    // This clears out the table and the view as well
    deinitContacts ();
    deinitInbox ();

    arrNumbers.clear ();

    actLogin.setText ("Login...");

    bLoggedIn = false;

    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_bIsLoggedIn", bLoggedIn);

    setStatus ("Logout complete");
    Q_DEBUG ("Logout complete");

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);

    if (token) {
        token->deleteLater ();
    }
}//MainWindow::logoutCompleted

void
MainWindow::setUsername(const QString &strU)
{
    do // Begin cleanup block (not a loop)
    {
        QObject *pSettingsPage = getQMLObject ("SettingsPage");
        if (NULL == pSettingsPage) {
            Q_WARN("Could not get to SettingsPage for setUsername");
            break;
        }

        QMetaObject::invokeMethod (pSettingsPage, "setUsername",
                                   Q_ARG (QVariant, QVariant(strU)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::setUsername

void
MainWindow::setPassword(const QString &strP)
{
    do // Begin cleanup block (not a loop)
    {
        QObject *pSettingsPage = getQMLObject ("SettingsPage");
        if (NULL == pSettingsPage) {
            Q_WARN("Could not get to SettingsPage for setPassword");
            break;
        }

        QMetaObject::invokeMethod (pSettingsPage, "setPassword",
                                   Q_ARG (QVariant, QVariant(strP)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::setPassword

void
MainWindow::onTwoStepAuthentication(AsyncTaskToken *token)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setTFAFlag (true);

    QObject *obj = getQMLObject ("MsgBox");
    if (NULL == obj) {
        gvApi.cancel (token);
        Q_ASSERT(obj);
        return;
    }

    inputBoxCtx = token;
    connect(this, SIGNAL(sigMessageBoxDone(bool)),
            this, SLOT(onPinMsgBoxDone(bool)));

    obj->setProperty ("inputText", "");
    showInputBox ("Enter security token");
}//MainWindow::onTwoStepAuthentication

void
MainWindow::onPinMsgBoxDone(bool ok)
{
    disconnect(this, SIGNAL(sigMessageBoxDone(bool)),
               this, SLOT(onPinMsgBoxDone(bool)));

    AsyncTaskToken *token = (AsyncTaskToken *)inputBoxCtx;
    inputBoxCtx = NULL;

    QObject *obj = getQMLObject ("MsgBox");

    do {// Begin cleanup block (not a loop)

        if (NULL == obj) {
            gvApi.cancel (token);
            Q_ASSERT(obj);
            break;
        }

        if (!ok) {
            gvApi.cancel (token);
            break;
        }

        token->inParams["user_pin"] = obj->property("inputText").toString();
        gvApi.resumeTFALogin (token);
        token = NULL;
    } while (0); // End cleanup block (not a loop)

    if (NULL != token) {
        token->deleteLater ();
    }
}//MainWindow::onPinMsgBoxDone

void
MainWindow::onAppPassMsgBoxDone(bool ok)
{
    disconnect(this, SIGNAL(sigMessageBoxDone(bool)),
               this, SLOT(onAppPassMsgBoxDone(bool)));

    do {// Begin cleanup block (not a loop)
        QObject *obj = getQMLObject ("MsgBox");
        if (NULL == obj) {
            Q_ASSERT(obj);
            break;
        }

        if (!ok) {
            Q_WARN("User refused to enter app specific pass for contacts");
            break;
        }

        QString contactsPass = obj->property("inputText").toString();

        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.setContactsPass (contactsPass);

        // Prepare the contacts
        initContacts (contactsPass);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onAppPassMsgBoxDone
