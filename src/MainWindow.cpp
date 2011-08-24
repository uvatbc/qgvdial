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

#include <phonon/AudioOutput>
#include <phonon/AudioOutputDevice>
#include <QDesktopServices>
#include <iostream>
using namespace std;

MainWindow::MainWindow (QWidget *parent)
: QDeclarativeView (parent)
, fLogfile (this)
, logLevel (5)
, icoQgv (":/qgv.png")
, pSystray (NULL)
, oContacts (this)
, oInbox (this)
, vmailPlayer (NULL)
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
, jar (NULL)
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

    rv = connect (&statusTimer, SIGNAL (timeout()),
                   this       , SLOT   (onStatusTimerTick ()));
    Q_ASSERT(rv);

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

    if (NULL != jar) {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        dbMain.saveCookies (jar);

        // The jar that I just deleted was a child of this class. That jar is
        // now gone. I cannot set NULL as the new cookie jar - that causes a
        // NULL deref in the Qt code. So, create a new cookie jar just for the
        // web page so that it can substitute the old pointer with the new one
        // and be happy.
        GVAccess &webPage = Singletons::getRef().getGVAccess ();
        webPage.nwAccessMgr ()->setCookieJar (new CookieJar);
        delete jar;
        jar = NULL;
    }

    // Dump the log one last time.
    QMutexLocker locker(&logMutex);
    for (int i = 0; i < arrLogTextStream.size(); i++) {
        // Send to log file
        if (fLogfile.isOpen ()) {
            QTextStream streamLog(&fLogfile);
            streamLog << arrLogTextStream[i] << endl;
        }
    }
    arrLogTextStream.clear ();
}//MainWindow::~MainWindow

/** Initialize the log file name and timer.
 * The log timer is required to update the log view every 3 seconds.
 * The log cannot be updated every time a log is entered because the view MUST
 * be updated only in the GUI thread. The timer always runs in the context of
 * the main thread - which is the GUI thread.
 */
void
MainWindow::initLogging ()
{
    // Initialize logging
    OsDependent &osd = Singletons::getRef().getOSD ();
    QString strLogfile = osd.getAppDirectory ();
    strLogfile += QDir::separator ();
    strLogfile += "qgvdial.log";
    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::WriteOnly | QIODevice::Append);

    logsTimer.setSingleShot (true);
    logsTimer.start (3 * 1000);
    bool rv = connect (&logsTimer, SIGNAL(timeout()),
                        this     , SLOT(onCleanupLogsArray()));
    Q_ASSERT(rv); Q_UNUSED(rv);

    setStatus("Using qgvdial version __QGVDIAL_VERSION__");
}//MainWindow::initLogging

/** Log information to console and to log file
 * This function is invoked from the qDebug handler that is installed in main.
 * @param strText Text to be logged
 * @param level Log level
 */
void
MainWindow::log (const QString &strText, int level /*= 10*/)
{
    QString strDisp;
    QRegExp regex("^\"(.*)\"\\s*");
    if (strText.indexOf (regex) != -1) {
        strDisp = regex.cap (1);
    } else {
        strDisp = strText;
    }

    if ((strDisp.contains ("password", Qt::CaseInsensitive)) ||
        ((!strPass.isEmpty ()) &&
         (strDisp.contains (strPass, Qt::CaseInsensitive)))) {
        strDisp = "Log message with password blocked";
    }

    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(strDisp);

    // Send to standard output
    cout << strLog.toStdString () << endl;

    if (level <= logLevel) {
        // Append it to the circular buffer
        QMutexLocker locker(&logMutex);
        arrLogMsgs.prepend (strLog);
        arrLogTextStream.append (strLog);
        bKickLocksTimer = true;
    }
}//MainWindow::log

void
MainWindow::onCleanupLogsArray()
{
    int timeout = 3 * 1000;
    do { // Begin cleanup block (not a loop)
        QMutexLocker locker(&logMutex);
        if (!bKickLocksTimer) {
            break;
        }
        bKickLocksTimer = false;
        timeout = 1 * 1000;

        for (int i = 0; i < arrLogTextStream.size(); i++) {
            // Send to log file
            if (fLogfile.isOpen ()) {
                QTextStream streamLog(&fLogfile);
                streamLog << arrLogTextStream[i] << endl;
            }
        }
        arrLogTextStream.clear ();

        while (arrLogMsgs.size () > 50) {
            arrLogMsgs.removeLast ();
        }

        QDeclarativeContext *ctx = this->rootContext();
        ctx->setContextProperty ("g_logModel", QVariant::fromValue(arrLogMsgs));
    } while (0); // End cleanup block (not a loop)

    logsTimer.start (timeout);
}//MainWindow::onCleanupLogsArray

/** Status update function
 * Use this function to update the status. The status is shown dependent on the
 * platform. On Windows and Linux, this status is shown on the system tray as a
 * notification message from our systray icon. On Maemo, it is shown as the
 * notification banner.
 * @param strText Text to show as the status
 * @param timeout Timeout in milliseconds. 0 indicates a status that remains
 *          until the next status is to be displayed.
 */
void
MainWindow::setStatus(const QString &strText, int timeout /* = 3000*/)
{
    qWarning () << strText;

#ifdef Q_WS_MAEMO_5
    infoBox.hide ();

    // Show the banner only if the window is invisible. Otherwise the QML
    // status bar is more than enough for this job.
    if (!this->isVisible ()) {
        QLabel *theLabel = (QLabel *) infoBox.widget ();
        if (NULL == theLabel) {
            theLabel = new QLabel (strText, &infoBox);
            theLabel->setAlignment (Qt::AlignHCenter);
            infoBox.setWidget (theLabel);
            qDebug("Created the Maemo5 yellow banner label");
        } else {
            qDebug() << "Display the status banner:" << strText;
            theLabel->setText (strText);
        }
        infoBox.setTimeout (0 == timeout ? 3000 : timeout);
        infoBox.show ();
    }
#else
    if (NULL != pSystray) {
        pSystray->showMessage ("Status", strText,
                               QSystemTrayIcon::Information,
                               timeout);
    }
#endif

    statusTimer.stop ();
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_strStatus", strText);

    if (0 != timeout) {
        statusTimer.setSingleShot (true);
        statusTimer.setInterval (timeout);
        statusTimer.start ();
    }
}//MainWindow::setStatus

void
MainWindow::onStatusTimerTick ()
{
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_strStatus", "Ready");
}//MainWindow::onStatusTimerTick

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
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    OsDependent &osd = Singletons::getRef().getOSD ();
    bool rv;

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
    jar = new CookieJar(this);
    dbMain.loadCookies (jar);
    webPage.nwAccessMgr()->setCookieJar (jar);
    jar->setParent (this);

    // The GV access class signals these during the dialling protocol
    rv = connect (&webPage    , SIGNAL (dialInProgress (const QString &)),
                       this       , SLOT   (dialInProgress (const QString &)));
    Q_ASSERT(rv);
    rv = connect ( this       , SIGNAL (dialCanFinish ()),
                      &webPage    , SLOT   (dialCanFinish ()));
    Q_ASSERT(rv);
    rv = connect (
        &webPage, SIGNAL (dialAccessNumber (const QString &,
                                            const QVariant &)),
         this   , SLOT   (dialAccessNumber (const QString &,
                                            const QVariant &)));
    Q_ASSERT(rv);

    // Skype client factory needs a main widget. Also, it needs a status sink.
    SkypeClientFactory &skypeFactory = Singletons::getRef().getSkypeFactory ();
    skypeFactory.setMainWidget (this);
    rv = connect (
        &skypeFactory, SIGNAL (status(const QString &, int)),
         this        , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);

    // Telepathy Observer factory init and status
    ObserverFactory &obF = Singletons::getRef().getObserverFactory ();
    obF.init ();
    rv = connect (&obF , SIGNAL (status(const QString &, int)),
                   this, SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);

    // webPage status
    rv = connect (&webPage, SIGNAL (status(const QString &, int)),
                   this   , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);

    // When the call initiators change, update us
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    rv = connect (&cif, SIGNAL(changed()),
                   this, SLOT(onCallInitiatorsChange()));
    Q_ASSERT(rv);
    // Call initiator status
    rv = connect (&cif , SIGNAL (status(const QString &, int)),
                   this, SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);

    // Status from contacts object
    rv = connect (&oContacts, SIGNAL (status   (const QString &, int)),
                   this     , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);
    // oContacts.allContacts -> this.getContactsDone
    rv = connect (&oContacts, SIGNAL (allContacts (bool)),
                   this      , SLOT   (getContactsDone (bool)));
    Q_ASSERT(rv);
    // Status from inbox object
    rv = connect (&oInbox, SIGNAL (status   (const QString &, int)),
                   this  , SLOT   (setStatus(const QString &, int)));
    Q_ASSERT(rv);

    // Inbox Model creation
    rv = connect (&oInbox, SIGNAL (setInboxModel(QAbstractItemModel *)),
                   this  , SLOT (onSetInboxModel(QAbstractItemModel *)));
    Q_ASSERT(rv);
    // Inbox selector changes
    rv = connect (&oInbox, SIGNAL (setInboxSelector(const QString &)),
                   this  , SLOT (onSetInboxSelector(const QString &)));
    Q_ASSERT(rv);

    // Inbox Model creation
    rv = connect (
        &oContacts, SIGNAL (setContactsModel(QAbstractItemModel *)),
         this     , SLOT (onSetContactsModel(QAbstractItemModel *)));
    Q_ASSERT(rv);

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
    rv = connect (&actDismiss, SIGNAL (triggered()),
                   this      , SLOT   (hide ()));
    Q_ASSERT(rv);
    rv = connect (&actRefresh, SIGNAL (triggered()),
                   this      , SLOT   (onRefresh()));
    Q_ASSERT(rv);
    rv = connect (&actExit, SIGNAL (triggered()),
                   this   , SLOT   (on_actionE_xit_triggered()));
    Q_ASSERT(rv);

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
    rv = connect (&mqThread , SIGNAL(sigUpdateContacts(const QDateTime &)),
                  &oContacts, SLOT  (refreshContacts(const QDateTime &)));
    Q_ASSERT(rv);
    rv = connect (&mqThread , SIGNAL(status(QString,int)),
                   this     , SLOT  (setStatus(QString,int)));
    Q_ASSERT(rv);
    rv = connect (&mqThread, SIGNAL(finished()),
                   this    , SLOT(onMqThreadFinished()));
    Q_ASSERT(rv);
#endif

    // If the cache has the username and password, begin login
    if (dbMain.getUserPass (strUser, strPass)) {
        this->setUsername (strUser);
        this->setPassword (strPass);

        QVariantList l;
        logoutCompleted (true, l);
        // Login without popping up the "enter user/pass" dialog
        doLogin ();
    } else {
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
    onDesktopResized ();

    qmlRegisterType<WebWidget>("org.qgvdial.WebWidget", 1, 0, "MyWebWidget");

    bool bTempFalse = false;
    int iTempZero = 0;

    // Prepare the glabally accessible variants for QML.
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

    // Initialize the QML view
    this->setSource (QUrl ("qrc:/Main.qml"));
    this->setResizeMode (QDeclarativeView::SizeRootObjectToView);

    this->setUsername ("example@gmail.com");
    this->setPassword ("hunter2 :p");

    // The root object changes when we reload the source. Pick it up again.
    QGraphicsObject *gObj = this->rootObject();

    bool bOk;
    // Connect all signals to slots in this class.
    bOk = connect (gObj, SIGNAL (sigCall (QString)),
                   this, SLOT   (dialNow (QString)));
    Q_ASSERT(bOk);
    bOk = connect (
        gObj, SIGNAL (sigText (const QString &, const QString &)),
        this, SLOT   (onSigText (const QString &, const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigVoicemail (QString)),
                   this, SLOT   (retrieveVoicemail (const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigVmailPlayback (int)),
                   this, SLOT   (onSigVmailPlayback (int)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigSelChanged (int)),
                   this, SLOT   (onRegPhoneSelectionChange (int)));
    Q_ASSERT(bOk);
    bOk = connect (gObj   , SIGNAL (sigInboxSelect (QString)),
                   &oInbox, SLOT   (onInboxSelected (const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj   , SIGNAL (sigMarkAsRead (QString)),
                   &oInbox, SLOT   (onSigMarkAsRead (const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigCloseVmail ()),
                   this, SLOT   (onSigCloseVmail ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigUserChanged (const QString &)),
                   this, SLOT   (onUserTextChanged (const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigPassChanged (const QString &)),
                   this, SLOT   (onPassTextChanged (const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigLogin ()),
                   this, SLOT   (doLogin ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigLogout ()),
                   this, SLOT   (doLogout ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigRefresh ()),
                   this, SLOT   (onRefresh ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigRefreshAll ()),
                   this, SLOT   (onRefreshAll ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigRefreshInbox ()),
                   this, SLOT   (onSigRefreshInbox ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigRefreshContacts ()),
                   this, SLOT   (onSigRefreshContacts ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigHide ()),
                   this, SLOT   (onSigHide ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigQuit ()),
                   this, SLOT   (on_actionE_xit_triggered ()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigLinkActivated (const QString &)),
                   this, SLOT   (onLinkActivated (const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (
        gObj, SIGNAL (sigProxyChanges(bool, bool, const QString &, int,
                                      bool, const QString &, const QString &)),
        this, SLOT (onSigProxyChanges(bool, bool, const QString &, int,
                                      bool, const QString &, const QString &)));
    bOk = connect (gObj, SIGNAL (sigProxyRefresh()),
                   this, SLOT (onSigProxyRefresh()));
    Q_ASSERT(bOk);
    bOk = connect (
        gObj, SIGNAL (sigMosquittoChanges(bool, const QString &, int,
                                          const QString &)),
        this, SLOT   (onSigMosquittoChanges(bool, const QString &, int,
                                            const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigMosquittoRefresh()),
                   this, SLOT   (refreshMqSettings()));
    Q_ASSERT(bOk);
    bOk = connect (
        gObj, SIGNAL(sigPinSettingChanges  (bool, const QString &)),
        this, SLOT  (onSigPinSettingChanges(bool, const QString &)));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL(sigPinRefresh()),
                   this, SLOT  (refreshPinSettings()));
    Q_ASSERT(bOk);
    bOk = connect (gObj, SIGNAL (sigMsgBoxDone(bool)),
                   this, SLOT (onSigMsgBoxDone(bool)));
    Q_ASSERT(bOk);
    bOk = connect (
        gObj      , SIGNAL  (sigSearchContacts(const QString &)),
        &oContacts, SLOT (onSearchQueryChanged(const QString &)));
    Q_ASSERT(bOk);

#if MOBILE_OS
    bOk = connect(qApp->desktop(), SIGNAL(resized(int)),
                  this           , SLOT(onDesktopResized()));
    Q_ASSERT(bOk);
#endif

#if DESKTOP_OS
    Qt::WindowFlags flags = this->windowFlags ();
    flags |= Qt::CustomizeWindowHint;
    flags &= ~(Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
               Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
               Qt::WindowCloseButtonHint);
    this->setWindowFlags (flags);
    this->setFixedSize(this->size());
#endif

}//MainWindow::initQML

/** Invoked to begin the login process.
 * We already have the username and password, so just start the login to the GV
 * website. The async completion routine is loginCompleted.
 */
void
MainWindow::doLogin ()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;

    bool bOk = false;
    do { // Begin cleanup block (not a loop)
        webPage.setTimeout(60);

        l += strUser;
        l += strPass;

        setStatus ("Logging in...", 0);

        osd.setLongWork (this, true);

        bool rv = connect(&webPage, SIGNAL(twoStepAuthentication(QString &)),
                           this   , SLOT(onTwoStepAuthentication(QString &)));
        Q_ASSERT(rv);

        // webPage.workCompleted -> this.loginCompleted
        if (!webPage.enqueueWork (GVAW_login, l, this,
                SLOT (loginCompleted (bool, const QVariantList &))))
        {
            rv = disconnect(&webPage, SIGNAL(twoStepAuthentication(QString &)),
                             this   , SLOT(onTwoStepAuthentication(QString &)));
            Q_ASSERT(rv);

            qWarning ("Login returned immediately with failure!");
            osd.setLongWork (this, false);
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk)
    {
        webPage.setTimeout(20);
        // Cleanup if any
        strUser.clear ();
        strPass.clear ();

        l.clear ();
        logoutCompleted (true, l);
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
MainWindow::loginCompleted (bool bOk, const QVariantList & /*varList*/)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    bool rv = disconnect(&webPage, SIGNAL(twoStepAuthentication(QString &)),
                          this   , SLOT(onTwoStepAuthentication(QString &)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    strSelfNumber.clear ();
    webPage.setTimeout(20);

    if (!bOk) {
        QVariantList l;
        logoutCompleted (true, l);

        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setLongWork (this, false);

        setStatus ("User login failed", 30*1000);
        QString strErr = webPage.getLastErrorString ();
        if (strErr.isEmpty ()) {
            strErr = "User login failed";
        }
        this->showMsgBox (strErr);

        QTimer::singleShot (500, this, SLOT(onRecreateCookieJar()));
    } else {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        setStatus ("User logged in");

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
        refreshMqSettings ();

        // Fill up the pin settings
        refreshPinSettings ();
    }
}//MainWindow::loginCompleted

void
MainWindow::doLogout ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    webPage.enqueueWork (GVAW_logout, l, this,
                         SLOT (logoutCompleted (bool, const QVariantList &)));

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, true);

    oContacts.deinitModel ();
    oInbox.deinitModel ();

#if MOSQUITTO_CAPABLE
    mqThread.setQuit ();
#endif
}//MainWindow::doLogout

void
MainWindow::logoutCompleted (bool, const QVariantList &)
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
    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);
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
    qApp->quit ();
}//MainWindow::on_actionE_xit_triggered

void
MainWindow::getContactsDone (bool bOk)
{
    if (!bOk)
    {
        this->showMsgBox ("Contacts retrieval failed");
        setStatus ("Contacts retrieval failed");
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

/** Convert a number and a key to more info into a structure with all the info.
 * @param strNumber The phone number
 * @param strNameLink The key to the associated information. This parameter is
 *          optional. If it is not present, then a dummy structure is created
 *          that has only the number as valid information.
 */
bool
MainWindow::getInfoFrom (const QString &strNumber,
                         const QString &strNameLink,
                         ContactInfo &info)
{
    info.init ();

    if (0 != strNameLink.size ())
    {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        info.strId = strNameLink;

        if (!dbMain.getContactFromLink (info))
        {
            this->showMsgBox ("Failed to get contact information");
            return (false);
        }

        for (int i = 0; i < info.arrPhones.size (); i++)
        {
            QString lhs = strNumber;
            QString rhs = info.arrPhones[i].strNumber;

            GVAccess::simplify_number (lhs);
            GVAccess::simplify_number (rhs);
            if (lhs == rhs)
            {
                info.selected = i;
                break;
            }
        }
    }
    else
    {
        info.strTitle = strNumber;
        PhoneInfo num;
        num.Type = PType_Unknown;
        num.strNumber = strNumber;
        info.arrPhones += num;
        info.selected = 0;
    }

    return (true);
}//MainWindow::getInfoFrom

bool
MainWindow::findInfo (const QString &strNumber, ContactInfo &info)
{
    bool rv = true;
    info.init ();

    QString strTrunc = strNumber;
    GVAccess::simplify_number (strTrunc, false);
    strTrunc.remove(' ').remove('+');

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (!dbMain.getContactFromNumber (strTrunc, info)) {
        qDebug ("Could not find info about this number. Using dummy info");
        info.strTitle = strNumber;
        PhoneInfo num;
        num.Type = PType_Unknown;
        num.strNumber = strNumber;
        info.arrPhones += num;
        info.selected = 0;
    } else {
        // Found it, now set the "selected" field correctly
        info.selected = 0;
        foreach (PhoneInfo num, info.arrPhones) {
            QString strNum = num.strNumber;
            GVAccess::simplify_number (strNum, false);
            strNum.remove(' ').remove('+');

            if (-1 != strNum.indexOf (strTrunc)) {
                break;
            }
            info.selected++;
        }

        if (info.selected >= info.arrPhones.size ()) {
            info.selected = 0;
            rv = false;
        }
    }

    return (rv);
}//MainWindow::findInfo

void
MainWindow::dialNow (const QString &strTarget)
{
    CalloutInitiator *ci;

    do { // Begin cleanup block (not a loop)
        if (!bLoggedIn) {
            setStatus ("User is not logged in yet. Cannot make any calls.");
            break;
        }
        if (strSelfNumber.isEmpty () || (strSelfNumber == "CLIENT_ONLY")) {
            qWarning ("Self number is not valid. Dial canceled");
            setStatus ("Account not configured");
            showMsgBox ("Account not configured");
            break;
        }

        QMutexLocker locker (&mtxDial);
        if (bCallInProgress) {
            setStatus ("Another call is in progress. Please try again later");
            break;
        }

        GVRegisteredNumber gvRegNumber;
        if (!getDialSettings (bDialout, gvRegNumber, ci)) {
            setStatus ("Unable to dial because settings are not valid.");
            break;
        }

        if (strTarget.isEmpty ()) {
            setStatus ("Cannot dial empty number");
            break;
        }

        QString strTest = strTarget;
        strTest.remove(QRegExp ("\\d*"))
               .remove(QRegExp ("\\s"))
               .remove('+')
               .remove('-');
        if (!strTest.isEmpty ()) {
            setStatus ("Cannot use numbers with special symbols or characters");
            break;
        }

        DialContext *ctx = new DialContext(strSelfNumber, strTarget, this);
        if (NULL == ctx) {
            setStatus ("Failed to dial out because of allocation problem");
            break;
        }
        bool rv = connect (ctx , SIGNAL(sigDialComplete(DialContext*,bool)),
                           this, SLOT(onSigDialComplete(DialContext*,bool)));
        Q_ASSERT(rv); Q_UNUSED(rv);

        GVAccess &webPage = Singletons::getRef().getGVAccess ();
        QVariantList l;
        l += strTarget;     // The destination number is common between the two
        l += QVariant::fromValue<void*>(ctx);

        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setLongWork (this, true);

        bCallInProgress = true;
        bDialCancelled = false;

        ctx->showMsgBox ();

        if (bDialout) {
            ctx->ci = ci;

            l += ci->selfNumber ();
            if (!webPage.enqueueWork (GVAW_dialOut, l, this,
                    SLOT (dialComplete (bool, const QVariantList &))))
            {
                setStatus ("Dialing failed instantly");
                bCallInProgress = bDialCancelled = false;
                fallbackDialout (ctx);
                break;
            }
        } else {
            l += gvRegNumber.strNumber;
            l += QString (gvRegNumber.chType);
            if (!webPage.enqueueWork (GVAW_dialCallback, l, this,
                    SLOT (dialComplete (bool, const QVariantList &))))
            {
                setStatus ("Dialing failed instantly");
                bCallInProgress = bDialCancelled = false;
                fallbackDialout (ctx);
                break;
            }
        }
    } while (0); // End cleanup block (not a loop)
}//MainWindow::dialNow

void
MainWindow::onSigText (const QString &strNumbers, const QString &strText)
{
    QStringList arrNumbers;
    arrNumbers = strNumbers.split (',');
    sendSMS (arrNumbers, strText);
}//MainWindow::onSigText

//! Invoked by the DBus Text server
/**
 * When the DBus Text server's text method is called, it finally reaches this
 * function. Here, we:
 * 1. Find out information (if there is any) about each number
 * 2. Add that info into the widget that collects numbers to send a text to
 * 3. Show the text widget
 */
void
MainWindow::onSendTextWithoutData (const QStringList &arrNumbers)
{
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for SmsPage");
            break;
        }

        QObject *pSmsView = pRoot->findChild <QObject*> ("SmsPage");
        if (NULL == pSmsView) {
            qWarning ("Could not get to SmsPage");
            break;
        }

        foreach (QString strNumber, arrNumbers) {
            if (strNumber.isEmpty ()) {
                qWarning ("Cannot text empty number");
                continue;
            }

            ContactInfo info;

            // Get info about this number
            if (!findInfo (strNumber, info)) {
                qWarning () << "Unable to find information for " << strNumber;
                continue;
            }

            QMetaObject::invokeMethod (pSmsView, "addSmsDestination",
                Q_ARG (QVariant, QVariant(info.strTitle)),
                Q_ARG (QVariant, QVariant(info.arrPhones[info.selected].strNumber)));
        }

        // Show the SMS View
        QMetaObject::invokeMethod (pRoot, "showSmsView");

        // Show myself (because I may be hidden)
        this->show ();
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onSendTextWithoutData

void
MainWindow::onSigDialComplete (DialContext *ctx, bool ok)
{
    // Disconnecting this
    QMutexLocker locker (&mtxDial);
    if (ok) {
        if (!ctx->bDialOut) {
            emit dialCanFinish ();
        }
    } else {
        GVAccess &webPage = Singletons::getRef().getGVAccess ();
        bDialCancelled = true;
        webPage.cancelWork (ctx->bDialOut ? GVAW_dialOut : GVAW_dialCallback);
    }
}//MainWindow::onSigDialComplete

void
MainWindow::dialInProgress (const QString & /*strNumber*/)
{
}//MainWindow::dialInProgress

void
MainWindow::dialAccessNumber (const QString  &strAccessNumber,
                              const QVariant &context        )
{
    bool bSuccess = false;
    DialContext *ctx = (DialContext *) context.value<void *>();
    do // Begin cleanup block (not a loop)
    {
        if (NULL == ctx)
        {
            setStatus ("Invalid call out context", 3);
            setStatus ("Callout failed");
            break;
        }

        if (NULL == ctx->ci)
        {
            qWarning ("Invalid call out initiator");
            setStatus ("Callout failed");
            break;
        }

        ctx->ci->initiateCall (strAccessNumber);
        setStatus ("Callout in progress");
        bSuccess = true;
    } while (0); // End cleanup block (not a loop)
}//MainWindow::dialAccessNumber

void
MainWindow::dialComplete (bool bOk, const QVariantList &params)
{
    QMutexLocker locker (&mtxDial);
    DialContext *ctx = (DialContext *) params[1].value <void*> ();
    bool bReleaseContext = true;

    if (!bOk) {
        if (bDialCancelled) {
            setStatus ("Cancelled dial out");
        } else if (NULL == ctx->fallbackCi) {
            // Not currently in fallback modem and there was a problem
            // ... so start fallback mode
            qDebug ("Attempting fallback dial");
            bReleaseContext = false;
            fallbackDialout (ctx);
        } else {
            setStatus ("Dialing failed", 10*1000);
            GVAccess &webPage = Singletons::getRef().getGVAccess ();
            QString strErr = webPage.getLastErrorString ();
            this->showMsgBox (strErr);
        }
    } else {
        setStatus (QString("Dial successful to %1.").arg(params[0].toString()));
    }
    bCallInProgress = false;

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);

    if (bReleaseContext) {
        ctx->deleteLater ();
    }
}//MainWindow::dialComplete

void
MainWindow::sendSMS (const QStringList &arrNumbers, const QString &strText)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QStringList arrFailed;
    QString msg;

    for (int i = 0; i < arrNumbers.size (); i++)
    {
        if (arrNumbers[i].isEmpty ()) {
            qWarning ("Cannot text empty number");
            continue;
        }

        QVariantList l;
        l += arrNumbers[i];
        l += strText;
        if (!webPage.enqueueWork (GVAW_sendSMS, l, this,
                SLOT (sendSMSDone (bool, const QVariantList &))))
        {
            arrFailed += arrNumbers[i];
            msg = QString ("Failed to send an SMS to %1").arg (arrNumbers[i]);
            qWarning () << msg;
            break;
        }
        clearSmsDestinations ();
    } // loop through all the numbers

    if (0 != arrFailed.size ())
    {
        this->showMsgBox (QString("Failed to send %1 SMS")
                                 .arg (arrFailed.size ()));
        msg = QString("Could not send a text to %1")
                .arg (arrFailed.join (", "));
        setStatus (msg);
    }
}//MainWindow::sendSMS

void
MainWindow::sendSMSDone (bool bOk, const QVariantList &params)
{
    QString msg;
    if (!bOk)
    {
        msg = QString("Failed to send SMS to %1").arg (params[0].toString());
    }
    else
    {
        msg = QString("SMS sent to %1").arg (params[0].toString());
    }

    setStatus (msg);
}//MainWindow::sendSMSDone

bool
MainWindow::refreshRegisteredNumbers ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if (!bLoggedIn)
        {
            qWarning ("Not logged in. Will not refresh registered numbers.");
            break;
        }

        arrNumbers.clear ();

        QVariantList l;
        rv = connect(
            &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
             this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
        Q_ASSERT(rv);
        if (!webPage.enqueueWork (GVAW_getRegisteredPhones, l, this,
                SLOT (gotAllRegisteredPhones (bool, const QVariantList &))))
        {
            rv = disconnect(
                &webPage,
                    SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
                 this   ,
                    SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
            Q_ASSERT(rv);
            rv = false;
            qWarning ("Failed to retrieve registered contacts!!");
            break;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//MainWindow::refreshRegisteredNumbers

void
MainWindow::gotRegisteredPhone (const GVRegisteredNumber &info)
{
    arrNumbers += info;
}//MainWindow::gotRegisteredPhone

void
MainWindow::gotAllRegisteredPhones (bool bOk, const QVariantList &params)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    bool rv = disconnect(
        &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
         this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
    Q_ASSERT(rv); Q_UNUSED(rv);

    do { // Begin cleanup block (not a loop)
        if (!bOk)
        {
            this->showMsgBox ("Failed to retrieve registered phones");
            setStatus ("Failed to retrieve registered phones");
            break;
        }

        // Save the users GV number returned by the login completion
        strSelfNumber = params[params.size()-1].toString ();

        this->onCallInitiatorsChange (true);

        setStatus ("GV callbacks retrieved.");
    } while (0); // End cleanup block (not a loop)
}//MainWindow::gotAllRegisteredPhones

bool
MainWindow::getDialSettings (bool                 &bDialout   ,
                             GVRegisteredNumber   &gvRegNumber,
                             CalloutInitiator    *&initiator  )
{
    initiator = NULL;

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        RegNumData data;
        if (!modelRegNumber.getAt (indRegPhone, data)) {
            qWarning ("Invalid registered phone index");
            break;
        }

        gvRegNumber.chType = data.chType;
        gvRegNumber.strName = data.strName;
        gvRegNumber.strNumber = data.strDesc;
        bDialout = (data.type == RNT_Callout);
        if (bDialout) {
            initiator = (CalloutInitiator *) data.pCtx;
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//MainWindow::getDialSettings

void
MainWindow::retrieveVoicemail (const QString &strVmailLink)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();

    do // Begin cleanup block (not a loop)
    {
        if (mapVmail.contains (strVmailLink))
        {
            setStatus ("Playing cached vmail");
            playVmail (mapVmail[strVmailLink]);
            break;
        }

        QString strTemplate = QDir::tempPath ()
                            + QDir::separator ()
                            + "qgv_XXXXXX.tmp.mp3";
        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ())
        {
            qWarning ("Failed to get a temp file name");
            break;
        }
        QString strTemp = QFileInfo (tempFile.fileName ()).absoluteFilePath ();
        tempFile.close ();

        QVariantList l;
        l += strVmailLink;
        l += strTemp;
        if (!webPage.enqueueWork (GVAW_playVmail, l, this,
                SLOT (onVmailDownloaded (bool, const QVariantList &))))
        {
            qWarning ("Failed to play Voice mail");
            break;
        }
    } while (0); // End cleanup block (not a loop)
}//MainWindow::retrieveVoicemail

void
MainWindow::onVmailDownloaded (bool bOk, const QVariantList &arrParams)
{
    QString strFilename = arrParams[1].toString ();
    if (bOk)
    {
        QString strVmailLink = arrParams[0].toString ();
        if (!mapVmail.contains (strVmailLink))
        {
            mapVmail[strVmailLink] = strFilename;
            setStatus ("Voicemail downloaded");
        }
        else
        {
            setStatus ("Voicemail already existed. Using cached vmail");
            if (strFilename != mapVmail[strVmailLink]) {
                QFile::remove (strFilename);
            }
        }

        playVmail (mapVmail[strVmailLink]);
    }
    else
    {
        QFile::remove (strFilename);
    }
}//MainWindow::onVmailDownloaded

void
MainWindow::playVmail (const QString &strFile)
{
    do // Begin cleanup block (not a loop)
    {
        // Convert it into a file:// url
        QUrl url = QUrl::fromLocalFile(strFile).toString ();

        qDebug() << "Play vmail file:" << strFile << "Url =" << url;

        createVmailPlayer ();
        vmailPlayer->setCurrentSource (Phonon::MediaSource(url));
//        vmailPlayer->setVolume (50);
        vmailPlayer->play();
    } while (0); // End cleanup block (not a loop)
}//MainWindow::playVmail

void
MainWindow::onVmailPlayerStateChanged(Phonon::State newState,
                                      Phonon::State /*oldState*/)
{
    int value = -1;
    qDebug() << "Vmail player state changed to" << newState;

    switch (newState) {
    case Phonon::StoppedState:
    case Phonon::ErrorState:
        value = 0;
        break;
    case Phonon::PlayingState:
        value = 1;
        break;
    case Phonon::PausedState:
        value = 2;
        break;
    default:
        qWarning ("Unknown state!");
        return;
    }

    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_vmailPlayerState", value);
}//MainWindow::onVmailPlayerStateChanged

void
MainWindow::onSigVmailPlayback (int newstate)
{
    if (NULL == vmailPlayer) {
        qDebug("Vmail object not available.");
        return;
    }

    switch(newstate) {
    case 0:
        qDebug ("QML asked us to stop vmail");
        vmailPlayer->stop ();
        break;
    case 1:
        qDebug ("QML asked us to play vmail");
        vmailPlayer->play ();
        break;
    case 2:
        qDebug ("QML asked us to pause vmail");
        vmailPlayer->pause ();
        break;
    default:
        qDebug() << "Unknown newstate =" << newstate;
        break;
    }
}//MainWindow::onSigVmailPlayback

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
    qDebug ("Refresh all requested.");

    refreshRegisteredNumbers ();
    oInbox.refresh ();
    oContacts.refreshContacts ();
}//MainWindow::onRefresh

void
MainWindow::onRefreshAll ()
{
    qDebug ("Refresh all requested.");

    refreshRegisteredNumbers ();
    oInbox.refreshFullInbox ();
    oContacts.refreshAllContacts ();
}//MainWindow::onRefreshAll

void
MainWindow::onSigRefreshContacts()
{
    qDebug ("Refresh Contacts requested.");

    oContacts.refreshContacts ();
}//MainWindow::onSigRefreshContacts

void
MainWindow::onSigRefreshInbox()
{
    qDebug ("Refresh inbox requested.");

    oInbox.refresh ();
}//MainWindow::onSigRefreshInbox

void
MainWindow::onSigProxyChanges(bool bEnable, bool bUseSystemProxy,
                              const QString &host, int port, bool bRequiresAuth,
                              const QString &user, const QString &pass)
{
    // Send to WebPage.
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    webPage.setProxySettings (bEnable, bUseSystemProxy, host, port,
                              bRequiresAuth, user, pass);

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
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for ProxySettingsPage");
            break;
        }

        QObject *pProxySettings = pRoot->findChild <QObject*>
                ("ProxySettingsPage");
        if (NULL == pProxySettings) {
            qWarning ("Could not get to ProxySettingsPage");
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
    if (!bForceShut) {
        if (!dbMain.getMqSettings (bEnable, host, port, topic)) {
            return;
        }
    }

    do { // Begin cleanup block (not a loop)
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for MosquittoPage");
            break;
        }

        QObject *pMqSettings = pRoot->findChild <QObject*> ("MosquittoPage");
        if (NULL == pMqSettings) {
            qWarning ("Could not get to MosquittoPage");
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
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for PinSettingsPage");
            break;
        }

        QObject *pPinSettings = pRoot->findChild <QObject*> ("PinSettingsPage");
        if (NULL == pPinSettings) {
            qWarning ("Could not get to PinSettingsPage");
            break;
        }

        QMetaObject::invokeMethod (pPinSettings, "setValues",
                                   Q_ARG (QVariant, QVariant(bEnable)),
                                   Q_ARG (QVariant, QVariant(strPin)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::refreshPinSettings

void
MainWindow::onMqThreadFinished ()
{
#if MOSQUITTO_CAPABLE
    if (bRunMqThread) {
        bRunMqThread = false;
        qDebug ("Finished waiting for Mq thread, restarting thread.");
        mqThread.start();
    } else {
        qDebug ("Finished waiting for Mq thread. Not restarting");

        // Just send a command to turn off mosquitto enable
        refreshMqSettings (true);
    }
#endif
}//MainWindow::onMqThreadFinished

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
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get QML root object for setUsername");
            break;
        }

        QObject *pSettingsPage = pRoot->findChild <QObject*>("SettingsPage");
        if (NULL == pSettingsPage) {
            qWarning ("Could not get to SettingsPage for setUsername");
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
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get QML root object for setPassword");
            break;
        }

        QObject *pSettingsPage = pRoot->findChild <QObject*>("SettingsPage");
        if (NULL == pSettingsPage) {
            qWarning ("Could not get to SettingsPage for setPassword");
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
        qWarning("Fallback dialout not possible. Self number not configured.");
        ctx->deleteLater ();
        return;
    }

    ctx->fallbackCi = cif.getFallbacks()[0];
    bool rv = connect (ctx->fallbackCi, SIGNAL(callInitiated(bool,void*)),
                      this,            SLOT  (onFallbackDialout(bool,void*)));
    Q_ASSERT(rv); Q_UNUSED(rv);
    GVAccess::simplify_number (strFull);
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
        qWarning ("Fallback dial failed. Aborting DTMF");
        setStatus ("Fallback dial failed", 10*1000);
        return;
    }

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
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for InboxPage");
            break;
        }

        QObject *pInbox = pRoot->findChild <QObject*> ("InboxPage");
        if (NULL == pInbox) {
            qWarning ("Could not get to InboxPage");
            break;
        }

        QMetaObject::invokeMethod (pInbox, "setSelector",
                                   Q_ARG (QVariant, QVariant(strSelector)));
    } while (0); // End cleanup block (not a loop)
}//MainWindow::onSetInboxSelector

void
MainWindow::onSetContactsModel(QAbstractItemModel *model)
{
    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_contactsModel", model);
}//MainWindow::onSetContactsModel

void
MainWindow::clearSmsDestinations ()
{
    do { // Begin cleanup block (not a loop)
        QObject *pRoot = this->rootObject ();
        if (NULL == pRoot) {
            qWarning ("Couldn't get root object in QML for SmsPage");
            break;
        }

        QObject *pSmsView = pRoot->findChild <QObject*> ("SmsPage");
        if (NULL == pSmsView) {
            qWarning ("Could not get to SmsPage");
            break;
        }

        QMetaObject::invokeMethod (pSmsView, "clearAllDestinations");
    } while (0); // End cleanup block (not a loop)
}//MainWindow::clearSmsDestinations

void
MainWindow::createVmailPlayer()
{
    onSigCloseVmail ();

    vmailPlayer = new Phonon::MediaObject(this);
    Phonon::AudioOutput *audioOutput =
                new Phonon::AudioOutput(Phonon::MusicCategory, vmailPlayer);
    Phonon::createPath(vmailPlayer, audioOutput);

    bool rv = connect (
        vmailPlayer, SIGNAL(stateChanged (Phonon::State, Phonon::State)),
        this, SLOT(onVmailPlayerStateChanged(Phonon::State, Phonon::State)));
    Q_ASSERT(rv); Q_UNUSED(rv);
}//MainWindow::createVmailPlayer

void
MainWindow::onSigCloseVmail()
{
    if (NULL != vmailPlayer) {
        vmailPlayer->deleteLater ();
        vmailPlayer = NULL;
    }
}//MainWindow::onSigCloseVmail

void
MainWindow::onRecreateCookieJar()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    jar->deleteLater ();
    jar = new CookieJar(this);
    webPage.nwAccessMgr()->setCookieJar (jar);
    jar->setParent (this);
}//MainWindow::onRecreateCookieJar

void
MainWindow::onTwoStepAuthentication(QString &result)
{
    int rv = QInputDialog::getInt (this, "Enter security token", "Token: ", 0, 0);
    result = QString("%1").arg (rv);
}//MainWindow::onTwoStepAuthentication

void
MainWindow::onDesktopResized()
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    QRect rect = osd.getStartingSize (this);

    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("g_MainWidth", rect.width ());
    ctx->setContextProperty ("g_MainHeight", rect.height ());
}//MainWindow::onDesktopResized
