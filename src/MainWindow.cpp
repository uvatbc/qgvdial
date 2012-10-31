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
#include "PhoneNumberValidator.h"

#include <QDesktopServices>
#include <iostream>
using namespace std;

MainWindow::MainWindow (QWidget *parent)
: QDeclarativeView (parent)
, gvApi (true, this)
, icoQgv (":/qgv.png")
, pSystray (NULL)
, oContacts (NULL)
, oInbox (NULL)
, bBeginPlayAfterLoad (false)
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
, loginStatus (LS_NotLoggedIn)
, loginTask(NULL)
, modelRegNumber (NULL)
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
, contactsTimer (this)
, inboxTimer (this)
{
    modelRegNumber = new RegNumberModel(this);
    if (modelRegNumber == NULL) {
        Q_WARN("Failed to allocate modelRegNumber");
        exit(1);
        return;
    }
    oContacts = new GVContactsTable(this);
    if (oContacts == NULL) {
        Q_WARN("Failed to allocate oContacts");
        exit(1);
        return;
    }
    oInbox = new GVInbox(gvApi, this);
    if (oInbox == NULL) {
        Q_WARN("Failed to allocate oInbox");
        exit(1);
        return;
    }

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

    rv = connect (qApp, SIGNAL (messageReceived(const QString &)),
                  this, SLOT   (messageReceived(const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    rv = connect (&statusTimer, SIGNAL (timeout()),
                   this       , SLOT   (onStatusTimerTick ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    rv = connect (&contactsTimer, SIGNAL(timeout()),
                  this, SLOT(onPeriodicContactsRefresh()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&inboxTimer, SIGNAL(timeout()),
                  this, SLOT(onPeriodicInboxRefresh()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Schedule the init a bit later so that the app.exec() can begin executing
    QTimer::singleShot (10, this, SLOT (init()));
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

    if (pSystray != NULL) {
        delete pSystray;
        pSystray = NULL;
    }

    if (modelRegNumber != NULL) {
        delete modelRegNumber;
        modelRegNumber = NULL;
    }

    if (oContacts != NULL) {
        delete oContacts;
        oContacts = NULL;
    }

    if (oInbox != NULL) {
        delete oInbox;
        oInbox = NULL;
    }

    Singletons::getRef().deinit();
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
        Q_DEBUG ("Second instance asked us to show");
        this->show ();
    } else if (message == "quit") {
        Q_DEBUG ("Second instance asked us to quit");
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
    gvApi.dbg_alwaysFailDialing (dbMain.dbgGetAlwaysFailDialing ());

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

/* In case I *ever* think of going back to the webPage way of doing things...
    rv = connect (this    , SIGNAL(dialCanFinish()),
                  &webPage, SLOT  (dialCanFinish()));
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
    rv = connect (oContacts, SIGNAL (status   (const QString &, int)),
                  this     , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    // oContacts->allContacts -> this.getContactsDone
    rv = connect (oContacts, SIGNAL (allContacts (bool)),
                  this     , SLOT   (getContactsDone (bool)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    // Status from inbox object
    rv = connect (oInbox, SIGNAL (status   (const QString &, int)),
                  this  , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Inbox Model creation
    rv = connect (oInbox, SIGNAL (setInboxModel(QAbstractItemModel *)),
                  this  , SLOT (onSetInboxModel(QAbstractItemModel *)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    // Inbox selector changes
    rv = connect (oInbox, SIGNAL (setInboxSelector(const QString &)),
                  this  , SLOT (onSetInboxSelector(const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    // Inbox Model creation
    rv = connect (oContacts, SIGNAL (setContactsModel(QAbstractItemModel *,
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
    oContacts->setTempStore(strTempStore);

#if MOSQUITTO_CAPABLE
    // Connect the signals from the Mosquitto thread
    rv = connect (&mqThread, SIGNAL(sigUpdateInbox(const QDateTime &)),
                  oInbox   , SLOT  (refresh(const QDateTime &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&mqThread, SIGNAL(sigUpdateContacts(const QDateTime &)),
                  oContacts, SLOT  (mqUpdateContacts(const QDateTime &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&mqThread, SIGNAL(status(QString,int)),
                   this    , SLOT  (setStatus(QString,int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (&mqThread, SIGNAL(finished()),
                   this    , SLOT(onMqThreadFinished()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
#endif

    // When there is a real change in the NW configuration, reset the nw access
    // manager on the GvAPI class object
    rv = connect(&Singletons::getRef().getNwInfo(), SIGNAL(realChange()),
                 &gvApi, SLOT(resetNwMgr()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

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
    ctx->setContextProperty ("g_bIsLoggedIn", bTempFalse);
    ctx->setContextProperty ("g_bShowLoginSettings", bTempFalse);
    ctx->setContextProperty ("g_strStatus", "Getting Ready");
    ctx->setContextProperty ("g_vmailPlayerState", iTempZero);
    ctx->setContextProperty ("g_logModel", QVariant::fromValue(arrLogMsgs));
    ctx->setContextProperty ("g_hMul", hMul);
    ctx->setContextProperty ("g_wMul", wMul);
    ctx->setContextProperty ("g_fontMul", fontMul);

    ctx->setContextProperty ("g_IsMobilePlatform",
                             MOBILE_OS ? !bTempFalse : bTempFalse);

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
    rv = connect (gObj, SIGNAL(sigCall(QString)),
                  this, SLOT  (dialNow(QString)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (
        gObj, SIGNAL (sigText(const QString&,const QString&)),
        this, SLOT   (onSigText(const QString&,const QString&)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigSelChanged(int)),
                  this, SLOT(onRegPhoneSelectionChange(int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigHide ()),
                  this, SLOT(onSigHide ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigQuit ()),
                  this, SLOT(on_actionE_xit_triggered ()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (
        gObj     , SIGNAL(sigSearchContacts(const QString &)),
        oContacts, SLOT(onSearchQueryChanged(const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    rv = connect (gObj, SIGNAL(sigRefreshContacts()),
                  oContacts, SLOT(refreshContacts()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigRefreshAllContacts()),
                  oContacts, SLOT(refreshAllContacts()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    rv = connect (gObj, SIGNAL(sigHaptic()),
                  this, SLOT(onBtnClickFroHapticFeedback()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    gObj = getQMLObject ("RegisteredPhonesView");
    if (NULL == gObj) {
        Q_WARN("Could not get to RegisteredPhonesView");
        requestQuit ();
        return;
    }
    rv = connect (gObj, SIGNAL (sigSelChanged (int)),
                  this, SLOT   (onRegPhoneSelectionChange (int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    gObj = getQMLObject ("MsgBox");
    if (NULL == gObj) {
        Q_WARN("Could not get to MsgBox");
        requestQuit ();
        return;
    }
    rv = connect (gObj, SIGNAL(sigOk()), this, SLOT(onSigMsgBoxOk()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigCancel()), this, SLOT(onSigMsgBoxCancel()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }

    gObj = getQMLObject ("InboxPage");
    if (NULL == gObj) {
        Q_WARN("Could not get to InboxPage");
        requestQuit ();
        return;
    }
    rv = connect (gObj  , SIGNAL(sigInboxSelect(QString)),
                  oInbox, SLOT(onInboxSelected(const QString&)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigVoicemail(QString)),
                  this, SLOT(retrieveVoicemail(const QString&)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigVmailPlayback(int)),
                  this, SLOT(onSigVmailPlayback(int)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj  , SIGNAL(sigMarkAsRead(QString)),
                  oInbox, SLOT(onSigMarkAsRead(const QString &)));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigRefreshInbox()),
                  oInbox, SLOT(refresh()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect (gObj, SIGNAL(sigRefreshAllInbox()),
                  oInbox, SLOT(refreshFullInbox()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect(gObj, SIGNAL(sigCloseVmail()), this, SLOT(onSigCloseVmail()));
    Q_ASSERT(rv);
    if (!rv) { exit(1); }
    rv = connect(gObj, SIGNAL(sigDeleteInboxEntry(const QString &)),
                 this, SLOT(onSigDeleteInboxEntry(const QString &)));
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
                    obj , SIGNAL(sigPinSettingChanges (bool, const QString &)),
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

        obj = getQMLObject ("LoginPage");
        if (obj == NULL) {
            Q_WARN("Could not find LoginPage");
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
        rv = connect (obj , SIGNAL (sigCancelLogin ()),
                      this, SLOT   (doCancelLogin ()));
        Q_ASSERT(rv);
        if (!rv) { break; }

        obj = getQMLObject ("RefreshSettingsPage");
        if (obj == NULL) {
            Q_WARN("Could not find RefreshSettingsPage");
            break;
        }
        rv = connect(obj, SIGNAL(sigRefreshChanges(bool,const QString&,const QString&,bool,const QString&,int,const QString&)),
                     this,SLOT(onSigRefreshChanges(bool,const QString&,const QString&,bool,const QString&,int,const QString&)));
        Q_ASSERT(rv);
        if (!rv) { break; }

        rv = connect (obj , SIGNAL (sigRefreshPeriodSettings()),
                      this, SLOT   (refreshPeriodSettings()));
        Q_ASSERT(rv);
        if (!rv) { break; }
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//MainWindow::connectSettingsSignals

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
        Q_DEBUG(QString("Delete vmail cached at %1").arg (i.value ()));
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
MainWindow::initContacts (const QString &contactsPass)
{
    oContacts->setUserPass (strUser, contactsPass);
    oContacts->loginSuccess ();
    oContacts->initModel ();
    oContacts->refreshContacts ();
}//MainWindow::initContacts

void
MainWindow::deinitContacts ()
{
    oContacts->deinitModel ();
    oContacts->loggedOut ();
}//MainWindow::deinitContacts

void
MainWindow::initInbox ()
{
    oInbox->loginSuccess ();
    oInbox->initModel ();
    oInbox->refresh ();
}//MainWindow::initInbox

void
MainWindow::deinitInbox ()
{
    oInbox->deinitModel ();
    oInbox->loggedOut ();
}//MainWindow::deinitInbox

bool
MainWindow::refreshRegisteredNumbers ()
{
    AsyncTaskToken *token = NULL;

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if (loginStatus != LS_LoggedIn) {
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
        gvApiProgressString = "Getting user phones";
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

    gvApiProgressString.clear ();
    token->deleteLater ();
}//MainWindow::gotAllRegisteredPhones

void
MainWindow::onRegPhoneSelectionChange (int index)
{
    QObject *regView = getQMLObject ("RegisteredPhonesView");
    Q_ASSERT(regView);
    QMetaObject::invokeMethod (regView, "setSelected", Q_ARG(QVariant, index));

    indRegPhone = index;

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.putCallback (QString("%1").arg (indRegPhone));

    RegNumData data;
    if (!modelRegNumber->getAt (indRegPhone, data)) {
        data.strName = "<Unknown>";
    }

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);

    QStringList phones;
    for (int i = 0; i < modelRegNumber->rowCount (); i++) {
        if (!modelRegNumber->getAt (i, data)) {
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
    oInbox->refresh ();
    oContacts->refreshContacts ();
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
    Q_DEBUG(QString("MainWindow: Link activated: %1").arg (strLink));
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
        QObject *obj = getQMLObject ("RefreshSettingsPage");
        if (NULL == obj) {
            Q_WARN("Could not get to RefreshSettingsPage");
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

        QMetaObject::invokeMethod (obj, "setMqValues",
                                   Q_ARG (QVariant, QVariant(bEnable)),
                                   Q_ARG (QVariant, QVariant(strHost)),
                                   Q_ARG (QVariant, QVariant(port)),
                                   Q_ARG (QVariant, QVariant(strTopic)));
    } while (0); // End cleanup block (not a loop)

}//MainWindow::refreshMqSettings

void
MainWindow::initMq()
{
    bool bEnable = false, bRefreshEnable, ok;
    QString host, topic;
    int port;
    quint32 contactsSec, inboxSec;
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();

    refreshPeriodSettings ();

    ok = dbMain.getMqSettings (bEnable, host, port, topic);
#if MOSQUITTO_CAPABLE
    Q_DEBUG(QString("Initially Mq is %1").arg(bEnable ? "enabled":"disabled"));
    if (ok && bEnable) {
        Q_DEBUG ("Start Mq thread.");
        mqThread.setSettings (bEnable, host, port);
        mqThread.start();
    }
#endif

    ok = dbMain.getRefreshSettings (bRefreshEnable, contactsSec, inboxSec);
    Q_ASSERT(contactsTimer.isActive() == inboxTimer.isActive());
    if (ok && bRefreshEnable && !contactsTimer.isActive()) {
        contactsTimer.start (contactsSec);
        inboxTimer.start (inboxSec);
    }
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
MainWindow::showMsgBox (const QString &strMessage, bool inputBox)
{
    QObject *obj = getQMLObject ("MsgBox");
    if (NULL == obj) {
        Q_ASSERT(obj);
        return;
    }

    obj->setProperty ("msgText", strMessage);
    obj->setProperty ("inputBox", inputBox);
    obj->setProperty ("opacity", QVariant(1.0));
}//MainWindow::showMsgBox

void
MainWindow::showMsgBox (const QString &strMessage)
{
    showMsgBox (strMessage, false);
}//MainWindow::showMsgBox

void
MainWindow::showInputBox (const QString &strMessage)
{
    showMsgBox (strMessage, true);
}//MainWindow::showInputBox

void
MainWindow::onSigMsgBoxDone (bool ok /* = true*/)
{
    QObject *obj = getQMLObject ("MsgBox");
    if (NULL == obj) {
        Q_ASSERT(obj);
        return;
    }

    obj->setProperty ("opacity", QVariant(0.0));

    emit sigMessageBoxDone(ok);
}//MainWindow::onSigMsgBoxDone

void
MainWindow::onSigMsgBoxOk()
{
    onSigMsgBoxDone(true);
}//MainWindow::onSigMsgBoxOk

void
MainWindow::onSigMsgBoxCancel()
{
    onSigMsgBoxDone(false);
}//MainWindow::onSigMsgBoxCancel

void
MainWindow::onCallInitiatorsChange (bool bSave)
{
    QObject *model = getQMLObject ("RegisteredPhonesModel");
    Q_ASSERT(model);

    // Set the correct callback
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QString strCallback;
    bool bGotCallback = dbMain.getCallback (strCallback);

    QVariantMap oneEntry;
    QScriptEngine scriptEngine;
    bool isChecked = false;

    QString strCiName;

    // Clear out all entries in the model...
    QMetaObject::invokeMethod (model, "clear");
    modelRegNumber->clear ();

    for (int i = 0; i < arrNumbers.size (); i++) {
        strCiName = "Dial back: " + arrNumbers[i].strName;
        modelRegNumber->insertRow (strCiName,
                                   arrNumbers[i].strNumber,
                                   arrNumbers[i].chType);

        oneEntry.clear ();
        oneEntry["entryText"] = strCiName;
        oneEntry["entryNumber"] = arrNumbers[i].strNumber;
        oneEntry["entryType"] = arrNumbers[i].chType;
        oneEntry["isChecked"] = isChecked;

        QMetaObject::invokeMethod (model, "append",
            Q_ARG(QScriptValue, scriptEngine.toScriptValue(oneEntry)));
    }

    // Store the callouts in the same widget as the callbacks
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    foreach (CalloutInitiator *ci, cif.getInitiators ()) {
        if (ci->isValid ()) {
            strCiName = "Dial out: " + ci->name ();
            modelRegNumber->insertRow (strCiName, ci->selfNumber (), ci);

            oneEntry.clear ();
            oneEntry["entryText"] = strCiName;
            oneEntry["entryNumber"] = ci->selfNumber ();
            oneEntry["entryType"] = 'O';
            oneEntry["isChecked"] = isChecked;

            QMetaObject::invokeMethod (model, "append",
                Q_ARG(QScriptValue, scriptEngine.toScriptValue(oneEntry)));
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
                       this,    SLOT(onFallbackCallInitiated(bool,void*)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    GVApi::simplify_number (strFull);

    Q_DEBUG(QString("Attempting fallback dial out to %1").arg(strFull));
    ctx->fallbackCi->initiateCall (strFull, ctx);
}//MainWindow::fallbackDialout

void
MainWindow::onFallbackCallInitiated (bool bSuccess, void *v_ctx)
{
    DialContext *ctx = (DialContext *) v_ctx;
    disconnect (ctx->fallbackCi, SIGNAL(callInitiated(bool,void*)),
                this,    SLOT(onFallbackCallInitiated(bool,void*)));

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
}//MainWindow::onFallbackCallInitiated

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
            Q_WARN(QString("Couldn't get root object in QML for %1")
                    .arg(pageName));
            break;
        }

        if (pRoot->objectName() == pageName) {
            pObj = pRoot;
            break;
        }

        pObj = pRoot->findChild <QObject*> (pageName);
        if (NULL == pObj) {
            Q_WARN(QString("Could not find page %1").arg (pageName));
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (pObj);
}//MainWindow::getQMLObject

void
MainWindow::onSigGvApiProgress(double percent)
{
    QString msg;

    if (gvApiProgressString.isEmpty ()) {
        msg = QString("Progress : %1%").arg(percent);
    } else {
        msg = QString("%1 : %2%").arg(gvApiProgressString).arg(percent);
    }
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

void
MainWindow::onBtnClickFroHapticFeedback()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.onBtnClickFroHapticFeedback ();
}//MainWindow::onBtnClickFroHapticFeedback

void
MainWindow::onSigDeleteInboxEntry(const QString &id)
{
   connect(this, SIGNAL(sigMessageBoxDone(bool)),
           this, SLOT(onUserAllowedDelete(bool)));
   inputBoxCtx = new QString(id);
   showMsgBox("Are you sure you want to delete this entry?");
}//MainWindow::onSigDeleteInboxEntry

void
MainWindow::onUserAllowedDelete(bool ok)
{
   disconnect(this, SIGNAL(sigMessageBoxDone(bool)),
              this, SLOT(onUserAllowedDelete(bool)));

   QString *id = (QString *)inputBoxCtx;
   inputBoxCtx = NULL;

   if (NULL == id) {
      Q_CRIT("Context = NULL!!");
      return;
   }

   if (ok) {
      Q_DEBUG("User confirmed deletion");
      oInbox->onSigDeleteInboxEntry(*id);
   } else {
      Q_DEBUG("User canceled deletion");
   }

   delete id;
}//MainWindow::onUserAllowedDelete

void
MainWindow::onSigRefreshChanges(bool bRefreshEnable,
                                const QString &contactsPeriod,
                                const QString &inboxPeriod, bool bMqEnable,
                                const QString &host, int port,
                                const QString &topic)
{
    quint32 contactsSec = contactsPeriod.toInt();
    quint32 inboxSec = inboxPeriod.toInt();

    if (contactsSec == 0) {
        contactsSec++;
    }
    if (inboxSec == 0) {
        inboxSec++;
    }
    contactsSec *= 60;
    inboxSec *= 60;

    // Save settings in seconds
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setRefreshSettings (bRefreshEnable, contactsSec, inboxSec);

    Q_ASSERT(contactsTimer.isActive() == inboxTimer.isActive());

    if (bRefreshEnable && !contactsTimer.isActive()) {
        contactsTimer.start (contactsSec);
        inboxTimer.start (inboxSec);
    }

    onSigMosquittoChanges (bMqEnable, host, port, topic);
}//MainWindow::onSigRefreshChanges

void
MainWindow::refreshPeriodSettings(bool bForceShut /*= false*/)
{
    refreshMqSettings (bForceShut);

    bool bEnable = bForceShut;
    quint32 contactsPeriod, inboxPeriod;

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (!dbMain.getRefreshSettings (bEnable, contactsPeriod, inboxPeriod)) {
        return;
    }

    // Convert to minutes before giving to the UI
    contactsPeriod /= 60;
    inboxPeriod /= 60;
    if (contactsPeriod == 0) {
        contactsPeriod++;
    }
    if (contactsPeriod == 0) {
        contactsPeriod++;
    }

    if (bForceShut) {
        bEnable = false;
    }

    do { // Begin cleanup block (not a loop)
        QObject *obj = getQMLObject ("RefreshSettingsPage");
        if (NULL == obj) {
            Q_WARN("Could not get to RefreshSettingsPage");
            break;
        }

        QMetaObject::invokeMethod (obj, "setRefreshValues",
                                   Q_ARG (QVariant, QVariant(bEnable)),
                                   Q_ARG (QVariant, QVariant(contactsPeriod)),
                                   Q_ARG (QVariant, QVariant(inboxPeriod)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::refreshPeriodSettings

void
MainWindow::onPeriodicContactsRefresh()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    bool bEnable;
    quint32 contactsPeriod, inboxPeriod;

    oContacts->refreshContacts ();

    if (!dbMain.getRefreshSettings (bEnable, contactsPeriod, inboxPeriod)) {
        return;
    }

    if (!bEnable) {
        return;
    }

    contactsTimer.start (contactsPeriod);
}//MainWindow::onPeriodicContactsRefresh

void
MainWindow::onPeriodicInboxRefresh()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    bool bEnable;
    quint32 contactsPeriod, inboxPeriod;

    oInbox->checkRecent ();

    if (!dbMain.getRefreshSettings (bEnable, contactsPeriod, inboxPeriod)) {
        return;
    }

    if (!bEnable) {
        return;
    }

    inboxTimer.start (inboxPeriod);
}//MainWindow::onPeriodicInboxRefresh
