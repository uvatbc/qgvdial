/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

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
#include "PhoneNumberValidator.h"

#include <QDesktopServices>
#include <iostream>
using namespace std;

MainWindow::MainWindow (QWidget *parent)
: QDeclarativeView (parent)
, gvApi (true, this)
, icoQgv (":/qgv.png")
, pSystray (NULL)
, oContacts (this)
, oInbox (gvApi, this)
, vmailPlayer (NULL)
, nwMgr (NULL)
, statusTimer (this)
#ifdef Q_WS_MAEMO_5
, infoBox (this)
#endif
, menuFile ("&File", this)
, actLogin ("Login...", this)
, actDismiss ("Dismiss", this)
, actRefresh ("Refresh", this)
, actExit ("Exit", this)
, bLoggedIn (false)
, modelRegNumber (this)
, indRegPhone (0)
, mtxDial (QMutex::Recursive)
, bCallInProgress (false)
, bDialCancelled (false)
, logMutex (QMutex::Recursive)
, logsTimer (this)
, bKickLocksTimer (false)
#if MOSQUITTO_CAPABLE
, mqThread (QString("qgvdial:%1").arg(QHostInfo::localHostName())
            .toLatin1().constData (), this)
#endif
, bQuitPath (false)
{
    initLogging ();

    qRegisterMetaType<ContactInfo>("ContactInfo");

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setDefaultWindowAttributes (this);

    initQML ();

    bool rv;
    // A systray icon if the OS supports it
    if (QSystemTrayIcon::isSystemTrayAvailable ())
    {
        pSystray = new QSystemTrayIcon (this);
        pSystray->setIcon (icoQgv);
        pSystray->setToolTip ("Google Voice dialer");
        pSystray->setContextMenu (&menuFile);
        rv = connect (
            pSystray,
            SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
            this,
            SLOT (systray_activated (QSystemTrayIcon::ActivationReason)));
        Q_ASSERT(rv);
        pSystray->show ();
    }

    rv = connect (qApp, SIGNAL (messageReceived (const QString &)),
                  this, SLOT   (messageReceived (const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    rv = connect (&statusTimer, SIGNAL (timeout()),
                   this       , SLOT   (onStatusTimerTick ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Schedule the init a bit later so that the app.exec() can begin executing
    QTimer::singleShot (100, this, SLOT (init()));
}//MainWindow::MainWindow

MainWindow::~MainWindow ()
{
#if MOSQUITTO_CAPABLE
    mqThread.terminate ();
#endif

    if (NULL != vmailPlayer) {
        delete vmailPlayer;
        vmailPlayer = NULL;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QList<QNetworkCookie> cookies = gvApi.getAllCookies ();
    dbMain.saveCookies (cookies);
}//MainWindow::~MainWindow

/** Invoked when the QtSingleApplication sends a message
 * We have used a QtSingleApplication to ensure that there is only one instance
 * of our program running in a specific user context. When the user attempts to
 * fire up another instance of our application, the second instance communicates
 * with the first and tells it to show the main window. the 2nd instance then
 * self-terminates. The first instance gets the "show" command as a parameter to
 * this SLOT.
 * It is also possible for the second instance to send a "quit" message. This is
 * almost always done to quit before uninstall/upgrade.
 * @param message The message passed by the other application.
 */
void
MainWindow::messageReceived (const QString &message)
{
    if (message == "show") {
        qDebug ("Second instance asked us to show");
        this->show ();
    } else if (message == "quit") {
        qDebug ("Second instance asked us to quit");
        this->on_actionE_xit_triggered ();
    }
}//MainWindow::messageReceived

/** Invoked when the user clicks the hide button or long presses the top bar
 * This function is supposed to hide the qgvdial main window. On all platforms
 * except Symbian a simple hide() is sufficient. In Symbian, we need to do it
 * differently.
 */
void
MainWindow::onSigHide ()
{
#if defined(Q_OS_SYMBIAN)
    // Symbian qgvdial needs lowering, not hiding.
    this->lower ();
#else
    this->hide ();
#endif
}//MainWindow::onSigHide

/** Deferred initialization function
 * This function does all of the initialization that was originally in the
 * constructor. It was moved out of the constructor because it takes a long time
 * to complete and because it was in the constructor, the GUI would not be shown
 * until the init was done. Since the GUI does not need full init, I shifted it
 * to this function and invoke this function function in a delay timer (100ms).
 * That way the constructor returns quickly, the app begins processing events,
 * the GUI is displayed and all is ready to show before init begins. Then as the
 * init progresses, the init functions can output status messages documenting
 * what the app is currently doing. User and programmer both happy!
 */
void
MainWindow::init ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    OsDependent &osd = Singletons::getRef().getOSD ();
    bool rv;

    Q_DEBUG ("Initializing...");
    setStatus ("Initializing...");

    // Initialize the database: This may create OR blowup and then re-create
    // the database.
    dbMain.init ();

    // Get the loglevel
    logLevel = dbMain.getLogLevel ();

    // Pick up proxy settings from the DB and apply to webpage.
    onSigProxyRefresh (true);

    // Initialize the DBUS interface to allow other applications (and qgv-tp) to
    // initiate calls and send texts through us.
    osd.initDialServer (this, SLOT (dialNow (const QString &)));
    osd.initTextServer (
        this, SLOT (sendSMS (const QStringList &, const QString &)),
        this, SLOT (onSendTextWithoutData (const QStringList &)));
    osd.initSettingsServer (
        this, SLOT(onRegPhoneSelectionChange(int)),
        this, SIGNAL(regPhoneChange(const QStringList &,int)));

    // Set up cookies
    QList<QNetworkCookie> cookies;
    dbMain.loadCookies (cookies);
    gvApi.setAllCookies (cookies);

    // The GV access class signals these during the dialling protocol
    rv = connect(&gvApi, SIGNAL(twoStepAuthentication(AsyncTaskToken *)),
                  this , SLOT(onTwoStepAuthentication(AsyncTaskToken *)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv =
    connect(&gvApi, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
             this , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    rv =
    connect(&gvApi, SIGNAL(sigProgress(double)),
             this , SLOT(onSigGvApiProgress(double)));
    Q_ASSERT(rv);

/*
    rv = connect ( this       , SIGNAL (dialCanFinish ()),
                      &webPage    , SLOT   (dialCanFinish ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
*/

    // Skype client factory needs a main widget. Also, it needs a status sink.
    SkypeClientFactory &skypeFactory = Singletons::getRef().getSkypeFactory ();
    skypeFactory.setMainWidget (this);
    rv = connect (
        &skypeFactory, SIGNAL (status(const QString &, int)),
         this        , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Telepathy Observer factory init and status
    ObserverFactory &obF = Singletons::getRef().getObserverFactory ();
    obF.init ();
    rv = connect (&obF , SIGNAL (status(const QString &, int)),
                   this, SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // When the call initiators change, update us
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    rv = connect (&cif, SIGNAL(changed()),
                   this, SLOT(onCallInitiatorsChange()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    // Call initiator status
    rv = connect (&cif , SIGNAL (status(const QString &, int)),
                   this, SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Status from contacts object
    rv = connect (&oContacts, SIGNAL (status   (const QString &, int)),
                   this     , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    // oContacts.allContacts -> this.getContactsDone
    rv = connect (&oContacts, SIGNAL (allContacts (bool)),
                   this     , SLOT   (getContactsDone (bool)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    // Status from inbox object
    rv = connect (&oInbox, SIGNAL (status   (const QString &, int)),
                   this  , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Inbox Model creation
    rv = connect (&oInbox, SIGNAL (setInboxModel(QAbstractItemModel *)),
                   this  , SLOT (onSetInboxModel(QAbstractItemModel *)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    // Inbox selector changes
    rv = connect (&oInbox, SIGNAL (setInboxSelector(const QString &)),
                   this  , SLOT (onSetInboxSelector(const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Inbox Model creation
    rv = connect (&oContacts, SIGNAL (setContactsModel(QAbstractItemModel *,
                                                       QAbstractItemModel *)),
                   this     , SLOT (onSetContactsModel(QAbstractItemModel *,
                                                       QAbstractItemModel *)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Additional UI initializations:
    //@@UV: Need this for later
//    ui->edNumber->setValidator (new PhoneNumberValidator (ui->edNumber));

    // Login/logout = Ctrl+L
    actLogin.setShortcut (QKeySequence(Qt::CTRL + Qt::Key_L));
#ifdef Q_WS_MAEMO_5
    // Dismiss = Esc
    actDismiss.setShortcut (QKeySequence(Qt::CTRL + Qt::Key_W));
#elif !MOBILE_OS
    // Dismiss = Esc
    actDismiss.setShortcut (QKeySequence(Qt::Key_Escape));
#endif
    // Refresh = Ctrl+R
    actRefresh.setShortcut (QKeySequence(Qt::CTRL + Qt::Key_R));
    // Quit = Ctrl+Q
    actExit.setShortcut (QKeySequence(Qt::CTRL + Qt::Key_Q));
    // Add these actions to the window
    menuFile.addAction (&actLogin);
    menuFile.addAction (&actDismiss);
    menuFile.addAction (&actRefresh);
    menuFile.addAction (&actExit);
    this->addAction (&actLogin);
    this->addAction (&actDismiss);
    this->addAction (&actRefresh);
    this->addAction (&actExit);
    // When the actions are triggered, do the corresponding work.
    rv = connect (&actLogin, SIGNAL (triggered()),
                   this    , SLOT   (on_action_Login_triggered()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&actDismiss, SIGNAL (triggered()),
                   this      , SLOT   (hide ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&actRefresh, SIGNAL (triggered()),
                   this      , SLOT   (onRefresh()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&actExit, SIGNAL (triggered()),
                   this   , SLOT   (on_actionE_xit_triggered()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    this->setWindowIcon (icoQgv);
    clearSmsDestinations ();

    // Save the temp store location for contact photos
    QString strTempStore = osd.getAppDirectory();
    QDir dirApp(strTempStore);
    strTempStore += QDir::separator() + tr("temp");
    if (!QFileInfo(strTempStore).exists ()) {
        dirApp.mkdir ("temp");
    }
    oContacts.setTempStore(strTempStore);

#if MOSQUITTO_CAPABLE
    // Connect the signals from the Mosquitto thread
    rv = connect (&mqThread , SIGNAL(sigUpdateInbox(const QDateTime &)),
                  &oInbox   , SLOT  (refresh(const QDateTime &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&mqThread , SIGNAL(sigUpdateContacts(const QDateTime &)),
                  &oContacts, SLOT  (mqUpdateContacts(const QDateTime &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&mqThread , SIGNAL(status(QString,int)),
                   this     , SLOT  (setStatus(QString,int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&mqThread, SIGNAL(finished()),
                   this    , SLOT(onMqThreadFinished()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
#endif

    // If the cache has the username and password, begin login
    if (dbMain.getUserPass (strUser, strPass)) {
        this->setUsername (strUser);
        this->setPassword (strPass);

        logoutCompleted (NULL);
        // Login without popping up the "enter user/pass" dialog
        doLogin ();
    } else {
        Q_DEBUG("No user and password set up. Asking user to enter that info");

        // Show this status for 60 seconds (or until the next status)
        setStatus ("Please enter email and password", 60 * 1000);

        strUser.clear ();
        strPass.clear ();

        on_action_Login_triggered ();
    }
}//MainWindow::init

void
MainWindow::initQML ()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    bool bTempFalse = false;
    int iTempZero = 0;

    double hMul, wMul, fontMul;
    osd.getMultipliers (hMul, wMul, fontMul);

    // Prepare the globally accessible variants for QML.
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_bShowMsg", bTempFalse);
    ctx->setContextProperty ("g_registeredPhonesModel", &modelRegNumber);
    ctx->setContextProperty ("g_bIsLoggedIn", bTempFalse);
    ctx->setContextProperty ("g_bShowSettings", bTempFalse);
    ctx->setContextProperty ("g_strStatus", "Getting Ready");
    ctx->setContextProperty ("g_strMsgText", "No message");
    ctx->setContextProperty ("g_CurrentPhoneName", "Not loaded");
    ctx->setContextProperty ("g_vmailPlayerState", iTempZero);
    ctx->setContextProperty ("g_logModel", QVariant::fromValue(arrLogMsgs));
    ctx->setContextProperty ("g_hMul", hMul);
    ctx->setContextProperty ("g_wMul", wMul);
    ctx->setContextProperty ("g_fontMul", fontMul);

    // Initialize the QML view
    this->setSource (QUrl(osd.getMainQML()));

    onOrientationChanged (osd.getOrientation ());
    this->setResizeMode (QDeclarativeView::SizeRootObjectToView);

    this->setUsername ("example@gmail.com");
    this->setPassword ("hunter2 :p");

    // The root object changes when we reload the source. Pick it up again.
    QObject *gObj = this->getQMLObject ("MainPage");
    if (NULL == gObj) {
        Q_WARN("Could not get to MainPage");
        requestQuit ();
        return;
    }

    bool rv;
    // Connect all signals to slots in this class.
    rv = connect (gObj, SIGNAL (sigCall (QString)),
                  this, SLOT   (dialNow (QString)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (
        gObj, SIGNAL (sigText (const QString &, const QString &)),
        this, SLOT   (onSigText (const QString &, const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL (sigVoicemail (QString)),
                  this, SLOT   (retrieveVoicemail (const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL (sigVmailPlayback (int)),
                  this, SLOT   (onSigVmailPlayback (int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL (sigSelChanged (int)),
                  this, SLOT   (onRegPhoneSelectionChange (int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj   , SIGNAL (sigInboxSelect (QString)),
                  &oInbox, SLOT   (onInboxSelected (const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj   , SIGNAL (sigMarkAsRead (QString)),
                  &oInbox, SLOT   (onSigMarkAsRead (const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL (sigCloseVmail ()),
                  this, SLOT   (onSigCloseVmail ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL (sigHide ()),
                  this, SLOT   (onSigHide ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL (sigQuit ()),
                  this, SLOT   (on_actionE_xit_triggered ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL (sigMsgBoxDone(bool)),
                  this, SLOT (onSigMsgBoxDone(bool)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (
        gObj      , SIGNAL  (sigSearchContacts(const QString &)),
        &oContacts, SLOT (onSearchQueryChanged(const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    rv = connect (gObj, SIGNAL (sigRefreshContacts()),
                  &oContacts, SLOT(refreshContacts()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigRefreshInbox()),
                  &oInbox,   SLOT(refresh()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Now do the settings page
    if (!connectSettingsSignals ()) {
        exit (1);
    }

    rv = connect (&osd, SIGNAL(orientationChanged(OsIndependentOrientation)),
                  this, SLOT(onOrientationChanged(OsIndependentOrientation)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

#if DESKTOP_OS
    Qt::WindowFlags flags = this->windowFlags ();
    flags |= Qt::CustomizeWindowHint;
    flags &= ~(Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint);
    this->setWindowFlags (flags);
    this->setFixedSize(this->size());

    gObj = getQMLObject ("CloseButton");
    gObj->setProperty ("visible", (bool)false);
#endif

}//MainWindow::initQML

bool
MainWindow::connectSettingsSignals()
{
    bool rv = false;
    QObject *obj = getQMLObject ("SettingsPage");

    do { // Begin cleanup block (not a loop)
        if (obj == NULL) {
            Q_WARN("Could not find SettingsPage");
            break;
        }
        rv = connect (obj , SIGNAL (sigUserChanged (const QString &)),
                      this, SLOT   (onUserTextChanged (const QString &)));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj , SIGNAL (sigPassChanged (const QString &)),
                      this, SLOT   (onPassTextChanged (const QString &)));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj , SIGNAL (sigLogin ()),
                      this, SLOT   (doLogin ()));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj , SIGNAL (sigLogout ()),
                      this, SLOT   (doLogout ()));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj , SIGNAL (sigLinkActivated (const QString &)),
                      this, SLOT   (onLinkActivated (const QString &)));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj,
                      SIGNAL (sigProxyChanges(bool, bool, const QString &, int,
                                              bool, const QString &,
                                              const QString &)),
                      this,
                      SLOT (onSigProxyChanges(bool, bool, const QString &, int,
                                              bool, const QString &,
                                              const QString &)));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj , SIGNAL (sigProxyRefresh()),
                      this, SLOT (onSigProxyRefresh()));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj,
                      SIGNAL (sigMosquittoChanges(bool, const QString &, int,
                                                  const QString &)),
                      this,
                      SLOT   (onSigMosquittoChanges(bool, const QString &, int,
                                                    const QString &)));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj , SIGNAL (sigMosquittoRefresh()),
                      this, SLOT   (refreshMqSettings()));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (
                    obj , SIGNAL(sigPinSettingChanges  (bool, const QString &)),
                    this, SLOT (onSigPinSettingChanges(bool, const QString &)));
        Q_ASSERT(rv);
        if (!rv) { break; }
        rv = connect (obj , SIGNAL(sigPinRefresh()),
                      this, SLOT  (refreshPinSettings()));
        Q_ASSERT(rv);

        rv = connect (obj , SIGNAL (sigSendLogs ()),
                      this, SLOT   (onSigSendLogs ()));
        Q_ASSERT(rv);
        if (!rv) { break; }
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//MainWindow::connectSettingsSignals

/** Invoked to begin the login process.
 * We already have the username and password, so just start the login to the GV
 * website. The async completion routine is loginCompleted.
 */
void
MainWindow::doLogin ()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    AsyncTaskToken *token = new AsyncTaskToken(this);
    bool bOk = false;

    do { // Begin cleanup block (not a loop)
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
    }
}//MainWindow::onUserTextChanged

void
MainWindow::onPassTextChanged (const QString &strPassword)
{
    if (strPass != strPassword) {
        strPass = strPassword;

        QList<QNetworkCookie> noCookies;
        gvApi.setAllCookies(noCookies);
        this->setPassword (strPass);
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
        ctx->setContextProperty ("g_bShowSettings", true);
    } else {
        doLogout ();
    }
}//MainWindow::on_action_Login_triggered

void
MainWindow::loginCompleted (AsyncTaskToken *token)
{
    if (!token || (token->status != ATTS_SUCCESS)) {
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

        Q_WARN("User login failed. Error string: ") << strErr;

        QTimer::singleShot (500, this, SLOT(onRecreateCookieJar()));
    } else {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        setStatus ("User logged in");
        Q_DEBUG ("User logged in");

        QString strOldUser, strOldPass;
        dbMain.getUserPass (strOldUser, strOldPass);
        if (strOldUser != strUser) {
            // Cleanup cache
            dbMain.blowAwayCache ();
            dbMain.ensureCache ();
        }

        // Prepare then contacts
        initContacts ();
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
        ctx->setContextProperty ("g_bShowSettings", bTemp);

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

    oContacts.deinitModel ();
    oInbox.deinitModel ();

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
MainWindow::systray_activated (QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        if (this->isVisible ()) {
            this->hide ();
        } else {
            this->show ();
        }
        break;

    default:
        break;
    }
}//MainWindow::systray_activated

void
MainWindow::on_actionE_xit_triggered ()
{
    this->close ();

    for (QMap<QString,QString>::iterator i  = mapVmail.begin ();
                                         i != mapVmail.end ();
                                         i++)
    {
        qDebug() << "Delete vmail cached at" << i.value ();
        QFile::remove (i.value ());
    }
    mapVmail.clear ();

#if MOSQUITTO_CAPABLE
    mqThread.setQuit ();
#endif

    requestQuit ();
}//MainWindow::on_actionE_xit_triggered

void
MainWindow::requestQuit()
{
    bQuitPath = true;
    qApp->quit ();

    QTimer::singleShot (1000, this, SLOT(dieNow()));
}//MainWindow::requestQuit

void
MainWindow::dieNow()
{
    exit(0);
}//MainWindow::dieNow

void
MainWindow::getContactsDone (bool bOk)
{
    if (!bOk) {
        this->showMsgBox ("Contacts retrieval failed");
        setStatus ("Contacts retrieval failed");
        Q_WARN ("Contacts retrieval failed");
    }
}//MainWindow::getContactsDone

void
MainWindow::initContacts ()
{
    oContacts.setUserPass (strUser, strPass);
    oContacts.loginSuccess ();
    oContacts.initModel ();
    oContacts.refreshContacts ();
}//MainWindow::initContacts

void
MainWindow::deinitContacts ()
{
    oContacts.deinitModel ();
    oContacts.loggedOut ();
}//MainWindow::deinitContacts

void
MainWindow::initInbox ()
{
    oInbox.loginSuccess ();
    oInbox.initModel ();
    oInbox.refresh ();
}//MainWindow::initInbox

void
MainWindow::deinitInbox ()
{
    oInbox.deinitModel ();
    oInbox.loggedOut ();
}//MainWindow::deinitInbox

bool
MainWindow::refreshRegisteredNumbers ()
{
    AsyncTaskToken *token = NULL;

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if (!bLoggedIn) {
            Q_WARN("Not logged in. Will not refresh registered numbers.");
            break;
        }

        arrNumbers.clear ();

        token = new AsyncTaskToken(this);
        if (!token) {
            Q_WARN("Allocation failure");
            break;
        }

        rv = connect (token, SIGNAL(completed(AsyncTaskToken*)),
                      this, SLOT(gotAllRegisteredPhones(AsyncTaskToken*)));
        Q_ASSERT(rv);

        rv = gvApi.getPhones (token);
        if (!rv) {
            Q_WARN("Get registered numbers failed at the start!!!");
            break;
        }
    } while (0); // End cleanup block (not a loop)

    if (!rv) {
        if (token) {
            delete token;
        }
    }

    return (rv);
}//MainWindow::refreshRegisteredNumbers

void
MainWindow::gotRegisteredPhone (const GVRegisteredNumber &info)
{
    arrNumbers += info;
}//MainWindow::gotRegisteredPhone

void
MainWindow::gotAllRegisteredPhones (AsyncTaskToken *token)
{
    do { // Begin cleanup block (not a loop)
        if (ATTS_SUCCESS != token->status) {
            this->showMsgBox ("Failed to retrieve registered phones");
            setStatus ("Failed to retrieve registered phones");
            Q_WARN ("Failed to retrieve registered phones");
            break;
        }

        this->onCallInitiatorsChange (true);

        Q_DEBUG("GV callbacks retrieved.");
        setStatus ("GV callbacks retrieved.");
    } while (0); // End cleanup block (not a loop)

    token->deleteLater ();
}//MainWindow::gotAllRegisteredPhones

void
MainWindow::onRegPhoneSelectionChange (int index)
{
    indRegPhone = index;

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.putCallback (QString("%1").arg (indRegPhone));

    RegNumData data;
    if (!modelRegNumber.getAt (indRegPhone, data)) {
        data.strName = "<Unknown>";
    }

    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_CurrentPhoneName", data.strName);

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);

    QStringList phones;
    for (int i = 0; i < modelRegNumber.rowCount (); i++) {
        if (!modelRegNumber.getAt (i, data)) {
            phones += "<Unknown>";
        } else {
            phones += data.strName;
        }
    }
    emit regPhoneChange(phones, index);
}//MainWindow::onRegPhoneSelectionChange

void
MainWindow::onRefresh ()
{
    Q_DEBUG ("Refresh all requested.");

    refreshRegisteredNumbers ();
    oInbox.refresh ();
    oContacts.refreshContacts ();
}//MainWindow::onRefresh

void
MainWindow::onSigProxyChanges(bool bEnable, bool bUseSystemProxy,
                              const QString &host, int port, bool bRequiresAuth,
                              const QString &user, const QString &pass)
{
    gvApi.setProxySettings (bEnable, bUseSystemProxy, host, port, bRequiresAuth,
                            user, pass);

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setProxySettings (bEnable, bUseSystemProxy, host, port,
                             bRequiresAuth, user, pass);
}//MainWindow::onSigProxyChanges

void
MainWindow::onSigProxyRefresh(bool bSet)
{
    bool bEnable, bUseSystemProxy, bRequiresAuth;
    int port;
    QString host, user, pass;
    CacheDatabase &dbMain = Singletons::getRef().getDBMain();

    dbMain.getProxySettings (bEnable, bUseSystemProxy, host, port,
                             bRequiresAuth, user, pass);

    if (bSet) {
        onSigProxyChanges (bEnable, bUseSystemProxy, host, port,
                           bRequiresAuth, user, pass);
    }

    do { // Begin cleanup block (not a loop)
        QObject *pProxySettings = getQMLObject ("ProxySettingsPage");
        if (NULL == pProxySettings) {
            Q_WARN("Could not get to ProxySettingsPage");
            break;
        }

        QMetaObject::invokeMethod (pProxySettings, "setValues",
                                   Q_ARG (QVariant, QVariant(bEnable)),
                                   Q_ARG (QVariant, QVariant(bUseSystemProxy)),
                                   Q_ARG (QVariant, QVariant(host)),
                                   Q_ARG (QVariant, QVariant(port)),
                                   Q_ARG (QVariant, QVariant(bRequiresAuth)),
                                   Q_ARG (QVariant, QVariant(user)),
                                   Q_ARG (QVariant, QVariant(pass)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onSigProxyRefresh

void
MainWindow::onLinkActivated (const QString &strLink)
{
    qDebug() << "MainWindow: Link activated" << strLink;
    QDesktopServices::openUrl (QUrl::fromUserInput (strLink));
}//MainWindow::onLinkActivated

void
MainWindow::onSigMosquittoChanges (bool bEnable, const QString &host, int port,
                                   const QString &topic)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setMqSettings (bEnable, host, port, topic);

#if MOSQUITTO_CAPABLE
    mqThread.setSettings (bEnable, host, port);
    bRunMqThread = bEnable;
    if (mqThread.isRunning ()) {
        mqThread.setQuit ();
    } else {
        onMqThreadFinished ();
    }
#endif
}//MainWindow::onSigMosquittoChanges

void
MainWindow::refreshMqSettings (bool bForceShut /*= false*/)
{
    bool bEnable = bForceShut;
    QString host, topic;
    int port;

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (!dbMain.getMqSettings (bEnable, host, port, topic)) {
        return;
    }

    if (bForceShut) {
        bEnable = false;
    }

    do { // Begin cleanup block (not a loop)
        QObject *pMqSettings = getQMLObject ("MosquittoPage");
        if (NULL == pMqSettings) {
            Q_WARN("Could not get to MosquittoPage");
            break;
        }

        QString strHost = host, strTopic = topic;
        if (host.isEmpty ()) {
            // This definitely does not exist.
            strHost = "mosquitto.example.com";
        }
        if (0 == port) {
            // Default mosquitto port
            port = 1883;
        }
        if (topic.isEmpty ()) {
            // Default topic
            strTopic = "gv_notify";
        }

        QMetaObject::invokeMethod (pMqSettings, "setValues",
                                   Q_ARG (QVariant, QVariant(bEnable)),
                                   Q_ARG (QVariant, QVariant(strHost)),
                                   Q_ARG (QVariant, QVariant(port)),
                                   Q_ARG (QVariant, QVariant(strTopic)));
    } while (0); // End cleanup block (not a loop)

}//MainWindow::refreshMqSettings

void
MainWindow::initMq()
{
    bool bEnable = false;
    QString host, topic;
    int port;

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (!dbMain.getMqSettings (bEnable, host, port, topic)) {
        return;
    }

    refreshMqSettings ();

    qDebug() << "Initially Mq is" << (bEnable ? "enabled" : "disabled");

#if MOSQUITTO_CAPABLE
    if (bEnable) {
        qDebug ("Start Mq thread.");
        mqThread.setSettings (bEnable, host, port);
        mqThread.start();
    }
#endif
}//MainWindow::initMq

void
MainWindow::onMqThreadFinished ()
{
    if (bQuitPath) {
        Q_DEBUG ("Quit path. Get out in a hurry");
        return;
    }

    if (bRunMqThread) {
        bRunMqThread = false;
#if MOSQUITTO_CAPABLE
        Q_DEBUG ("Finished waiting for Mq thread, restarting thread.");
        mqThread.start();
#endif
    } else {
        Q_DEBUG ("Finished waiting for Mq thread. Not restarting");

        // Just send a command to turn off mosquitto enable
        refreshMqSettings (true);
    }
}//MainWindow::onMqThreadFinished

void
MainWindow::onSigPinSettingChanges(bool bEnable, const QString &pin)
{
    QString strPin = QString("%1").arg (pin.toInt (), 4, 10, QChar('0'));

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setGvPin (bEnable, strPin);
}//MainWindow::onSigPinSettingChanges

void
MainWindow::refreshPinSettings()
{
    bool bEnable;
    QString strPin;

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (!dbMain.getGvPin (bEnable, strPin)) {
        return;
    }

    if (bEnable) {
        strGvPin = strPin;
    } else {
        strGvPin = "0000";
    }

    do { // Begin cleanup block (not a loop)
        QObject *pPinSettings = getQMLObject ("PinSettingsPage");
        if (NULL == pPinSettings) {
            Q_WARN("Could not get to PinSettingsPage");
            break;
        }

        QMetaObject::invokeMethod (pPinSettings, "setValues",
                                   Q_ARG (QVariant, QVariant(bEnable)),
                                   Q_ARG (QVariant, QVariant(strPin)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::refreshPinSettings

void
MainWindow::showMsgBox (const QString &strMessage)
{
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_bShowMsg", true);
    ctx->setContextProperty ("g_strMsgText", strMessage);
}//MainWindow::showMsgBox

void
MainWindow::onSigMsgBoxDone (bool /*ok*/)
{
    QDeclarativeContext *ctx = this->rootContext();
    bool bTemp = false;
    ctx->setContextProperty ("g_bShowMsg", bTemp);
}//MainWindow::onSigMsgBoxDone

void
MainWindow::onCallInitiatorsChange (bool bSave)
{
    // Set the correct callback
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QString strCallback;
    bool bGotCallback = dbMain.getCallback (strCallback);

    QString strCiName;
    modelRegNumber.clear ();
    for (int i = 0; i < arrNumbers.size (); i++)
    {
        strCiName = "Dial back: " + arrNumbers[i].strName;
        modelRegNumber.insertRow (strCiName,
                                  arrNumbers[i].strNumber,
                                  arrNumbers[i].chType);
    }

    // Store the callouts in the same widget as the callbacks
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    foreach (CalloutInitiator *ci, cif.getInitiators ()) {
        if (ci->isValid ()) {
            strCiName = "Dial out: " + ci->name ();
            modelRegNumber.insertRow (strCiName, ci->selfNumber (), ci);
        }
    }

    if (bGotCallback) {
        indRegPhone = strCallback.toInt ();
    }

    if (bSave)
    {
        // Save all callbacks into the cache
        dbMain.putRegisteredNumbers (arrNumbers);
    }

    onRegPhoneSelectionChange (indRegPhone);
}//MainWindow::onCallInitiatorsChange

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
MainWindow::fallbackDialout (DialContext *ctx)
{
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    if (cif.getFallbacks().length () < 1) {
        ctx->deleteLater ();
        this->showMsgBox ("Dialing failed");
        setStatus ("No fallback dial methods", 10*1000);
        return;
    }

    QString strFull = ctx->strMyNumber;
    if (strFull.isEmpty() || (strFull == "CLIENT_ONLY")) {
        Q_WARN("Fallback dialout not possible. Self number not configured.");
        ctx->deleteLater ();
        return;
    }

    ctx->fallbackCi = cif.getFallbacks()[0];
    bool rv = connect (ctx->fallbackCi, SIGNAL(callInitiated(bool,void*)),
                       this,          SLOT(onFallbackDialout(bool,void*)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    GVApi::simplify_number (strFull);

    Q_DEBUG(QString("Attempting fallback dial out to %1").arg(strFull));
    ctx->fallbackCi->initiateCall (strFull, ctx);
}//MainWindow::fallbackDialout

void
MainWindow::onFallbackDialout (bool bSuccess, void *v_ctx)
{
    DialContext *ctx = (DialContext *) v_ctx;
    disconnect (ctx->fallbackCi, SIGNAL(callInitiated(bool,void*)),
                this,            SLOT  (onFallbackDialout(bool,void*)));

    if (!bSuccess) {
        this->showMsgBox ("Dialing failed");
        Q_WARN("Fallback dial failed. Aborting DTMF");
        setStatus ("Fallback dial failed", 10*1000);
        return;
    }

    Q_DEBUG("Fallback dial is in progress. Now send DTMF");

    QString strDTMF;
    strDTMF = "p*p2p" + ctx->strTarget;
    // Add the pin if it is there
    if (!strGvPin.isEmpty ()) {
        strDTMF = strGvPin + "p" + strDTMF;
    }
    strDTMF = "p" + strDTMF + "#";

    ctx->fallbackCi->sendDTMF (strDTMF);

    ctx->deleteLater ();
}//MainWindow::onFallbackDialout

void
MainWindow::onSetInboxModel(QAbstractItemModel *model)
{
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_inboxModel", model);
}//MainWindow::onSetInboxModel

void
MainWindow::onSetInboxSelector(const QString &strSelector)
{
    do { // Begin cleanup block (not a loop)
        QObject *pInbox = getQMLObject ("InboxPage");
        if (NULL == pInbox) {
            Q_WARN("Could not get to InboxPage");
            break;
        }

        QMetaObject::invokeMethod (pInbox, "setSelector",
                                   Q_ARG (QVariant, QVariant(strSelector)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onSetInboxSelector

void
MainWindow::onSetContactsModel(QAbstractItemModel *model,
                               QAbstractItemModel *searchModel)
{
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_contactsModel", model);
    ctx->setContextProperty ("g_contactsSearchModel", searchModel);
}//MainWindow::onSetContactsModel

void
MainWindow::onRecreateCookieJar()
{
    QList<QNetworkCookie> empty;
    gvApi.setAllCookies (empty);
}//MainWindow::onRecreateCookieJar

void
MainWindow::onTwoStepAuthentication(AsyncTaskToken *token)
{
    int rv = QInputDialog::getInt (this, "Enter security token", "Token: ", 0, 0);
    token->inParams["user_pin"] = QString("%1").arg (rv);
}//MainWindow::onTwoStepAuthentication

void
MainWindow::onOrientationChanged(OsIndependentOrientation /*o*/)
{
    QObject *pMain = this->rootObject ();
    if (NULL == pMain) {
        Q_WARN("Could not get to MainPage for resize");
        return;
    }

    OsDependent &osd = Singletons::getRef().getOSD ();
    QRect rect = osd.getStartingSize ();
    pMain->setProperty("height", rect.height());
    pMain->setProperty("width", rect.width());
}//MainWindow::resetWindowSize

QObject *
MainWindow::getQMLObject(const char *pageName)
{
    QObject *pObj = NULL;
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            Q_WARN("Couldn't get root object in QML for ") << pageName;
            break;
        }

        if (pRoot->objectName() == pageName) {
            pObj = pRoot;
            break;
        }

        pObj = pRoot->findChild <QObject*> (pageName);
        if (NULL == pObj) {
            Q_WARN("Could not get to ") << pageName;
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (pObj);
}//MainWindow::getQMLObject

void
MainWindow::onSigGvApiProgress(double percent)
{
    QString msg = QString("Progress : %1%").arg (percent);
    setStatus (msg);
}//MainWindow::onSigGvApiProgress

bool
MainWindow::ensureNwMgr()
{
    if (nwMgr) {
        return true;
    }

    nwMgr = new QNetworkAccessManager(this);

    return (nwMgr != NULL);
}//MainWindow::ensureNwMgr
