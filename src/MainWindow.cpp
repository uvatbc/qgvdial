#include "MainWindow.h"
#include "Singletons.h"
#include "DlgSelectContactNumber.h"
#include "DialCancelDlg.h"

#include <iostream>
using namespace std;

MainWindow::MainWindow (QWidget *parent/* = 0*/, Qt::WindowFlags f/* = 0*/)
: QMainWindow(parent, f)
, fLogfile(this)
, tabMain(NULL)
, txtLogs(NULL)
, progressBar(NULL)
, btnCancel(NULL)
, gridMain(NULL)
, icoGoogle(":/Google.png")
, icoSkype(":/Skype.png")
, icoSMS(":/SMS.png")
, icoPhone(":/Phone.png")
, pSystray(NULL)
, actSettings ("Settings", this)
, actExit ("Exit", this)
, dlgSMS (this, ChildWindowBase_flags)
, vmailPlayer (this, ChildWindowBase_flags)
, stateMachine(this)
, wakeupTimer (this)
{
    initLogging ();

#ifdef Q_WS_MAEMO_5
    this->setAttribute (Qt::WA_Maemo5StackedWindow);
#endif

    dlgSMS.setWindowFlags (dlgSMS.windowFlags () | Qt::Window);
    vmailPlayer.hide ();

    UniqueAppHelper &unique = Singletons::getRef().getUAH ();
    QObject::connect (&unique, SIGNAL (log(const QString &, int)),
                      this   , SLOT   (log(const QString &, int)));

    GVAccess &webPage = Singletons::getRef().getGVAccess ();

    // Skype client factory log and status
    SkypeClientFactory &skypeFactory = Singletons::getRef().getSkypeFactory ();
    skypeFactory.setMainWidget (this);
    QObject::connect (
        &skypeFactory, SIGNAL (log(const QString &, int)),
         this        , SLOT   (log(const QString &, int)));
    QObject::connect (
        &skypeFactory, SIGNAL (status(const QString &, int)),
         this        , SLOT   (setStatus(const QString &, int)));

    // Observer factory log and status
    ObserverFactory &obF = Singletons::getRef().getObserverFactory ();
    obF.init ();
    QObject::connect (&obF , SIGNAL (log(const QString &, int)),
                       this, SLOT   (log(const QString &, int)));
    QObject::connect (&obF , SIGNAL (status(const QString &, int)),
                       this, SLOT   (setStatus(const QString &, int)));

    // webPage log and status
    QObject::connect (&webPage, SIGNAL (log(const QString &, int)),
                       this   , SLOT   (log(const QString &, int)));
    QObject::connect (&webPage, SIGNAL (status(const QString &, int)),
                       this   , SLOT   (setStatus(const QString &, int)));

    // call initiator log status and init
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    QObject::connect (&cif , SIGNAL (log(const QString &, int)),
                       this, SLOT   (log(const QString &, int)));
    QObject::connect (&cif , SIGNAL (status(const QString &, int)),
                       this, SLOT   (setStatus(const QString &, int)));

    // sInit.exited -> this.init
    // This trick is so that immediately after init we go to next state
    QObject::connect (&sInit, SIGNAL (exited ()),
                       this , SLOT   (init()));
    sInit.addTransition (&sInit, SIGNAL (entered ()), &sNotLoggedIn);

    // sNotLoggedIn.entered -> this.enterNotLoggedIn
    QObject::connect (&sNotLoggedIn, SIGNAL (entered ()),
                       this        , SLOT   (enterNotLoggedIn()));
    sNotLoggedIn.addTransition (this, SIGNAL (loginSuccess ()), &sLoggedIn);
    // sNotLoggedIn.exited -> this.exitNotLoggedIn
    QObject::connect (&sNotLoggedIn, SIGNAL (exited ()),
                       this        , SLOT   (exitNotLoggedIn()));

    // sNotLoggedIn.entered -> this.autoLogin (first time only)
    QObject::connect (&sNotLoggedIn, SIGNAL (entered ()),
                       this        , SLOT   (autoLogin()));

    // sLoggedIn.entered -> this.enterLoggedIn
    QObject::connect (&sLoggedIn, SIGNAL (entered ()),
                       this     , SLOT   (enterLoggedIn()));
    sLoggedIn.addTransition (this, SIGNAL (loggedOut ()), &sNotLoggedIn);
    // sLoggedIn.exited -> this.exitLoggedIn
    QObject::connect (&sLoggedIn, SIGNAL (exited ()),
                       this     , SLOT   (exitLoggedIn()));

    stateMachine.addState (&sInit);
    stateMachine.addState (&sNotLoggedIn);
    stateMachine.addState (&sLoggedIn);
    stateMachine.addState (&sFinal);
    stateMachine.setInitialState (&sInit);
    stateMachine.start ();
}//MainWindow::MainWindow

MainWindow::~MainWindow ()
{
}//MainWindow::~MainWindow

void
MainWindow::deinit ()
{
    for (QMap<QString,QString>::iterator i  = mapVmail.begin ();
                                         i != mapVmail.end ();
                                         i++)
    {
        QFile::remove (i.value ());
    }
    mapVmail.clear ();

    wakeupTimer.stop ();

    if (NULL != tabMain)
    {
        delete tabMain;
        tabMain = NULL;

        pDialer = NULL;
        pContactsView = NULL;
        pGVHistory = NULL;
    }

    qApp->quit ();
}//MainWindow::deinit

void
MainWindow::initLogging ()
{
    QString strLogfile = QDir::homePath ();
    if (!strLogfile.endsWith (QDir::separator ()))
    {
        strLogfile += QDir::separator ();
    }
    strLogfile += "qgvdial.log";

    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::WriteOnly | QIODevice::Append);
}//MainWindow::initLogging

void
MainWindow::log (const QString &strText, int level/* = 10*/)
{
    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(strText);
    if (NULL != txtLogs)
    {
        txtLogs->appendPlainText (strLog);
    }
    cout << strLog.toStdString () << endl;
    if (fLogfile.isOpen ())
    {
        QTextStream streamLog(&fLogfile);
        streamLog << strLog << endl;
    }
}//MainWindow::log

void
MainWindow::setStatus(const QString &strText, int timeout/* = 0*/)
{
    log (strText);
    this->statusBar()->showMessage (strText, timeout);
}//MainWindow::setStatus

void
MainWindow::showProgress ()
{
    gridMain->addWidget (progressBar, POS_BELOW_TAB  ,0, 1,3);
    gridMain->addWidget (btnCancel  , POS_BELOW_TAB  ,3);
    progressBar->show ();
    btnCancel->show ();
}//MainWindow::showProgress

void
MainWindow::hideProgress ()
{
    gridMain->removeWidget (progressBar);
    progressBar->hide();
    btnCancel->hide ();
}//MainWindow::hideProgress

void
MainWindow::init ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    OsDependent &osd = Singletons::getRef().getOSD ();

    dbMain.init ();
    osd.initDialServer (this, SLOT (dialNow (const QString &)));

    this->setWindowIcon (icoGoogle);
    if (QSystemTrayIcon::isSystemTrayAvailable ())
    {
        pSystray = new QSystemTrayIcon (this);
        pSystray->setIcon (icoGoogle);
        pSystray->setToolTip ("Google Voice dialer");
        QObject::connect (
            pSystray,
            SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
            this,
            SLOT (systray_activated (QSystemTrayIcon::ActivationReason)));
        pSystray->show ();
    }

    gridMain    = new QGridLayout (this->centralWidget ());
    progressBar = new QProgressBar (this->centralWidget ());
    tabMain     = new QTabWidget (this->centralWidget ());
    btnCancel   = new QPushButton ("Cancel", this->centralWidget ());

    progressBar->hide ();
    btnCancel->hide ();

    // Webview tab
#if !NO_DBGINFO
    webView = new MyWebView();
    webPage.setView (webView);
    tabMain->addTab (webView, "Web View");
#endif

    // Dialer
    pDialer = new DialerWidget ();
    QObject::connect (pDialer, SIGNAL (call (const QString &)),
                      this   , SLOT   (dialNow (const QString &)));
    tabMain->addTab (pDialer, "Dialer");

    // Contacts
    pContactsView = new GVContactsTable ();
    tabMain->addTab (pContactsView, "Contacts");

    // GV History
    pGVHistory = new GVHistory ();
    tabMain->addTab (pGVHistory, "History");

    // GV settings
    pGVSettings = new GVSettings (this, ChildWindowBase_flags);
    pGVSettings->hide ();

    gridMain->addWidget (tabMain, 0,0, 1,4);
#if !NO_DBGINFO
    txtLogs = new QPlainTextEdit (this->centralWidget ());
    gridMain->addWidget (txtLogs, POS_BELOW_TAB-1,0, 1,4);
#endif

    tabMain->setTabsClosable (false);
    tabMain->setTabPosition (QTabWidget::West);

    // This action needs to be added to the menubar ALWAYS
    QMenuBar *mbar = this->menuBar ();
    mbar->clear ();
    mbar->addAction (&actSettings);
    mbar->addAction (&actExit);

    // tabMain.currentChanged -> this.currentChanged
    QObject::connect (tabMain, SIGNAL (currentChanged (int)),
                      this   , SLOT   (tabChanged (int)));

    //- This is for progress bar related updates -//
    QObject::connect (&webPage    , SIGNAL (loadProgress (int)),
                       progressBar, SLOT   (setValue (int)));
    QObject::connect (&webPage    , SIGNAL (loadStarted ()),
                       this       , SLOT   (showProgress ()));
    QObject::connect (&webPage    , SIGNAL (loadFinished (bool)),
                       this       , SLOT   (hideProgress ()));

    // btnCancel.clicked -> webPage.userCancel
    QObject::connect ( btnCancel, SIGNAL (clicked ()),
                      &webPage  , SLOT   (userCancel ()));

    // actSettings.triggered -> pGVSettings.show
    QObject::connect (&actSettings, SIGNAL (triggered ()),
                       pGVSettings, SLOT   (show ()));
    QObject::connect (&actExit, SIGNAL (triggered ()),
                       this   , SLOT   (deinit ()));

    // Dialing handshake
    QObject::connect (&webPage    , SIGNAL (dialInProgress (const QString &)),
                       this       , SLOT   (dialInProgress (const QString &)));
    QObject::connect ( this       , SIGNAL (dialCanFinish ()),
                      &webPage    , SLOT   (dialCanFinish ()));
    QObject::connect (
        &webPage, SIGNAL (dialAccessNumber (const QString &,
                                            const QVariant &)),
         this   , SLOT   (dialAccessNumber (const QString &,
                                            const QVariant &)));

    // pContactsView.allContacts -> this.getContactsDone
    QObject::connect (pContactsView, SIGNAL (allContacts (bool)),
                      this         , SLOT   (getContactsDone (bool)));

    // pGVSettings.login -> this.doLogin
    QObject::connect (
        pGVSettings, SIGNAL (login   (const QString &, const QString &)),
        this       , SLOT   (doLogin (const QString &, const QString &)));
    // this.loginSuccess -> pGVSettings.loginDone
    QObject::connect (this       , SIGNAL (loginSuccess ()),
                      pGVSettings, SLOT   (loginDone ()));
    // pGVSettings.logout -> this.doLogout
    QObject::connect (pGVSettings, SIGNAL (logout ()),
                      this       , SLOT   (doLogout ()));
    // pGVSettings.newUser -> this.beginGetAccountDetails
    QObject::connect (pGVSettings, SIGNAL (newUser ()),
                      this       , SLOT   (beginGetAccountDetails ()));

    // pContactsView.call -> this.call
    QObject::connect (
        pContactsView, SIGNAL(callNumber(const QString &, const QString &)),
        this         , SLOT  (callNumber(const QString &, const QString &)));
    // pContactsView.SMS -> this.SMS
    QObject::connect (
        pContactsView, SIGNAL (textANumber (const QString &, const QString &)),
        this         , SLOT   (textANumber (const QString &, const QString &)));

    // pGVHistory.call -> this.call
    QObject::connect (
        pGVHistory, SIGNAL (callNumber (const QString &, const QString &)),
        this      , SLOT   (callNumber (const QString &, const QString &)));
    // pGVHistory.SMS -> this.SMS
    QObject::connect (
        pGVHistory, SIGNAL (textANumber (const QString &, const QString &)),
        this      , SLOT   (textANumber (const QString &, const QString &)));
    // pGVHistory.playVoicemail -> this.playVoicemail
    QObject::connect (
        pGVHistory, SIGNAL (playVoicemail (const QString &)),
        this      , SLOT   (playVoicemail (const QString &)));

    // dlgSMS.sendSMS -> this.sendSMS
    QObject::connect (
        &dlgSMS, SIGNAL (sendSMS (const QStringList &, const QString &)),
         this  , SLOT   (sendSMS (const QStringList &, const QString &)));

    // Logged in and logged out for all
    QObject::connect (this         , SIGNAL (loginSuccess ()),
                      pContactsView, SLOT   (loginSuccess ()));
    QObject::connect (this         , SIGNAL (loggedOut ()),
                      pContactsView, SLOT   (loggedOut ()));
    QObject::connect (this      , SIGNAL (loginSuccess ()),
                      pGVHistory, SLOT   (loginSuccess ()));
    QObject::connect (this      , SIGNAL (loggedOut ()),
                      pGVHistory, SLOT   (loggedOut ()));

    // Logging and setStatus
    QObject::connect (pDialer, SIGNAL (log(const QString &, int)),
                      this   , SLOT   (log(const QString &, int)));
    QObject::connect (pDialer, SIGNAL (status(const QString &, int)),
                      this   , SLOT   (setStatus(const QString &, int)));
    QObject::connect (pContactsView, SIGNAL (log(const QString &, int)),
                      this         , SLOT   (log(const QString &, int)));
    QObject::connect (pContactsView, SIGNAL (status(const QString &, int)),
                      this         , SLOT   (setStatus(const QString &, int)));
    QObject::connect (pGVSettings, SIGNAL (log(const QString &, int)),
                      this       , SLOT   (log(const QString &, int)));
    QObject::connect (pGVSettings, SIGNAL (status(const QString &, int)),
                      this       , SLOT   (setStatus(const QString &, int)));
    QObject::connect (pGVHistory, SIGNAL (log(const QString &, int)),
                      this      , SLOT   (log(const QString &, int)));
    QObject::connect (pGVHistory, SIGNAL (status(const QString &, int)),
                      this      , SLOT   (setStatus(const QString &, int)));
    QObject::connect (&vmailPlayer, SIGNAL (log(const QString &, int)),
                       this       , SLOT   (log(const QString &, int)));
    QObject::connect (&vmailPlayer, SIGNAL (status(const QString &, int)),
                       this       , SLOT   (setStatus(const QString &, int)));
    this->setCentralWidget (new QWidget (this));
    this->centralWidget()->setLayout (gridMain);

    // Set up the timer
    QObject::connect (&wakeupTimer, SIGNAL (timeout()),
                       this       , SLOT   (wakeupTimedOut()));
    wakeupTimer.setInterval (3*1000);
    wakeupTimer.setSingleShot (true);
    wakeupTimer.start ();
}//MainWindow::init

void
MainWindow::autoLogin ()
{
    QObject::disconnect (&sNotLoggedIn, SIGNAL (entered ()),
                          this        , SLOT   (autoLogin()));
    pGVSettings->causeLogin ();
}//MainWindow::autoLogin

void
MainWindow::enterNotLoggedIn ()
{
    pGVSettings->logoutDone (true);
    strSelfNumber.clear ();
}//MainWindow::enterNotLoggedIn

void
MainWindow::aboutBlankDone (bool)
{
}//MainWindow::aboutBlankDone

void
MainWindow::doLogin (const QString &strU, const QString &strP)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        pContactsView->setUserPass (strU, strP);

        QVariantList l;
        l += strU;
        l += strP;

        log ("Beginning login");
        // webPage.workCompleted -> this.loginCompleted
        if (!webPage.enqueueWork (GVAW_login, l, this,
                SLOT (loginCompleted (bool, const QVariantList &))))
        {
            log ("Login returned immediately with failure!", 3);
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk)
    {
        // Cleanup if any
        enterNotLoggedIn ();
    }
}//MainWindow::doLogin

void
MainWindow::loginCompleted (bool bOk, const QVariantList &varList)
{
    if (!bOk)
    {
        // Cleanup if any
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                           "Invalid username or password",
                           "Username or password not recognized",
                           QMessageBox::Close,
                           this);
        msgBox->setModal (false);
        QObject::connect (
            msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
            this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
        msgBox->show ();
        enterNotLoggedIn ();
    }
    else
    {
        strSelfNumber = varList[varList.size()-1].toString ();
        emit loginSuccess ();
    }
}//MainWindow::loginCompleted

void
MainWindow::exitNotLoggedIn ()
{
}//MainWindow::exitNotLoggedIn

void
MainWindow::enterLoggedIn ()
{
    // Model init
    pContactsView->initModel ();
    pGVHistory->initModel ();

    // Do this the first time (always)
    pGVHistory->refreshHistory ();
}//MainWindow::enterLoggedIn

void
MainWindow::beginGetAccountDetails ()
{
    pContactsView->refreshContacts ();
    pGVSettings->refreshRegisteredNumbers ();
}//MainWindow::beginGetAccountDetails

void
MainWindow::getContactsDone (bool bOk)
{
    if (!bOk)
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                           "Error",
                           "Contacts retrieval failed",
                           QMessageBox::Close);
        msgBox->setModal (false);
        QObject::connect (
            msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
            this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
        msgBox->show ();
        setStatus ("Contacts retrieval failed");
    }
}//MainWindow::getContactsDone

void
MainWindow::doLogout ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    webPage.enqueueWork (GVAW_logout, l, this,
                         SLOT (logoutCompleted (bool, const QVariantList &)));
}//MainWindow::doLogout

void
MainWindow::logoutCompleted (bool, const QVariantList &)
{
    // This clears out the table and the view as well
    pContactsView->deinitModel ();
    pGVHistory->deinitModel ();

    emit loggedOut ();

    setStatus ("Logout complete");
}//MainWindow::logoutCompleted

void
MainWindow::exitLoggedIn ()
{
}//MainWindow::exitLoggedIn

bool
MainWindow::getInfoFrom (const QString &strNumber,
                         const QString &strNameLink,
                         GVContactInfo &info)
{
    info = GVContactInfo();

    if (0 != strNameLink.size ())
    {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        info.strLink = strNameLink;

        if (!dbMain.getContactFromLink (info))
        {
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                                        "Get Contact failure",
                                        "Failed to get contact information",
                                        QMessageBox::Close,
                                        this);
            msgBox->setModal (false);
            QObject::connect (
                msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
                this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
            msgBox->show ();

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
        info.strName = strNumber;
        GVContactNumber num;
        num.chType = 'O';
        num.strNumber = strNumber;
        info.arrPhones += num;
        info.selected = 0;
    }

    return (true);
}//MainWindow::getInfoFrom

void
MainWindow::callNumber (const QString &strNumber,
                        const QString &strNameLink)
{
    GVContactInfo info;

    if (!getInfoFrom (strNumber, strNameLink, info))
    {
        return;
    }

    callWithContactInfo (info, false);
}//MainWindow::callNumber

void
MainWindow::callWithContactInfo (const GVContactInfo &info, bool bSaveIt)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (bSaveIt)
    {
        dbMain.putContactInfo (info);
    }
    else
    {
        log ("Got contact info from cached location");
    }

    DlgSelectContactNumber dlg(info, this);
    int rv = dlg.exec ();
    do // Begin cleanup block (not a loop)
    {
        if (QDialog::Accepted != rv)
        {
            log ("User canceled call", 3);
            break;
        }
        rv = dlg.getSelection ();
        if (-1 == rv)
        {
            log ("Invalid selection", 3);
            break;
        }

        dialNow (info.arrPhones[rv].strNumber);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::callWithContactInfo

void
MainWindow::contactsLinkWorkDone (bool, const QVariantList &)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
         this   , SLOT   (callWithContactInfo (const GVContactInfo &)));

    setStatus ("Retrieved contact info");
}//MainWindow::contactsLinkWorkDone

void
MainWindow::dialNow (const QString &strTarget)
{
    bool bDialout;
    CalloutInitiator *ci;

    do // Begin cleanup block (not a loop)
    {
        GVRegisteredNumber gvRegNumber;
        if (!pGVSettings->getDialSettings (bDialout, gvRegNumber, ci))
        {
            setStatus ("Unable to dial out because settings are not valid");
            break;
        }

        GVAccess &webPage = Singletons::getRef().getGVAccess ();
        QVariantList l;
        l += strTarget;     // The destination number is common between the two

        if (bDialout)
        {
            l += ci->selfNumber ();
            l += QVariant::fromValue<void*>(ci) ;
            if (!webPage.enqueueWork (GVAW_dialOut, l, this,
                            SLOT (dialComplete (bool, const QVariantList &))))
            {
                setStatus ("Dialing failed instantly");
            }
        }
        else
        {
            l += gvRegNumber.strNumber;
            l += QString (gvRegNumber.chType);
            if (!webPage.enqueueWork (GVAW_dialCallback, l, this,
                            SLOT (dialComplete (bool, const QVariantList &))))
            {
                setStatus ("Dialing failed instantly");
            }
        }
    } while (0); // End cleanup block (not a loop)
}//MainWindow::dialNow

void
MainWindow::dialInProgress (const QString &strNumber)
{
    bDialCancelled = false;

    DialCancelDlg msgBox(strNumber, this);
    int ret = msgBox.doModal (strSelfNumber);
    if (QMessageBox::Ok == ret)
    {
        emit dialCanFinish ();
    }
    else
    {
        bDialCancelled = true;
        GVAccess &webPage = Singletons::getRef().getGVAccess ();
        webPage.cancelWork (GVAW_dialCallback);
    }
}//MainWindow::dialInProgress

void
MainWindow::dialAccessNumber (const QString  &strAccessNumber,
                              const QVariant &context        )
{
    CalloutInitiator *ci = (CalloutInitiator *) context.value<void *>();
    if (NULL != ci)
    {
        ci->initiateCall (strAccessNumber);
        setStatus ("Callout in progress");
    }
    else
    {
        log ("Invalid call out initiator", 3);
        setStatus ("Callout failed");
    }
}//MainWindow::dialAccessNumber

void
MainWindow::dialComplete (bool bOk, const QVariantList &params)
{
    if (!bOk)
    {
        if (bDialCancelled)
        {
            setStatus ("Cancelled dial out");
        }
        else
        {
            setStatus ("Dialing failed", 3);
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                               "Dial failure",
                               "Dialing failed",
                               QMessageBox::Close,
                               this);
            msgBox->setModal (false);
            QObject::connect (
                msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
                this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
            msgBox->show ();
        }
    }
    else
    {
        setStatus (QString("Dial successful to %1.").arg(params[0].toString()));
    }
}//MainWindow::dialComplete

void
MainWindow::regNumChangeComplete (bool bOk, const QVariantList &)
{
    if (!bOk)
    {
        log ("Failed to save the callback number", 3);
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                           "Setting save failure",
                           "Failed to save the callback number",
                           QMessageBox::Close,
                           this);
        msgBox->setModal (false);
        QObject::connect (
            msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
            this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
        msgBox->show ();
    }
    else
    {
        setStatus ("Callback saved");
    }
}//MainWindow::regNumChangeComplete

void
MainWindow::msgBox_buttonClicked (QAbstractButton *button)
{
    if (NULL != button->parent ())
    {
        button->parent()->deleteLater ();
    }
}//MainWindow::msgBox_buttonClicked

void
MainWindow::systray_activated (QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        if (this->isVisible ())
        {
            this->hide ();
        }
        else
        {
            this->show ();
        }
        break;
    default:
        break;
    }
}//MainWindow::systray_activated

void
MainWindow::tabChanged (int index)
{
    QWidget *widget = tabMain->widget (index);
    QMenuBar *mbar = this->menuBar ();
    mbar->clear ();
    //TODO: Add menu items common to all tabs

    QObject::connect (this  , SIGNAL (updateMenu (QMenuBar *)),
                      widget, SLOT   (updateMenu (QMenuBar *)));
    emit updateMenu (mbar);
    QObject::disconnect (this  , SIGNAL (updateMenu (QMenuBar *)),
                         widget, SLOT   (updateMenu (QMenuBar *)));

    mbar->addAction (&actSettings);
    mbar->addAction (&actExit);
}//MainWindow::tabChanged

void
MainWindow::textANumber (const QString &strNumber,
                         const QString &strNameLink)
{
    GVContactInfo info;

    if (!getInfoFrom (strNumber, strNameLink, info))
    {
        return;
    }

    sendTextToContact (info, false);
}//MainWindow::textANumber

void
MainWindow::sendTextToContact (const GVContactInfo &info, bool bSaveIt)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (bSaveIt)
    {
        dbMain.putContactInfo (info);
    }
    else
    {
        log ("Got contact info from cached location");
    }

    DlgSelectContactNumber dlg(info, this);
    int rv = dlg.exec ();
    do // Begin cleanup block (not a loop)
    {
        if (QDialog::Accepted != rv)
        {
            log ("User canceled SMS", 3);
            break;
        }
        rv = dlg.getSelection ();
        if (-1 == rv)
        {
            log ("Invalid selection", 3);
            break;
        }

        SMSEntry entry;
        entry.strName = info.strName;
        entry.sNumber = info.arrPhones[rv];

        dlgSMS.addSMSEntry (entry);

        if (dlgSMS.isHidden ())
        {
            dlgSMS.show ();
        }
    } while (0); // End cleanup block (not a loop)
}//MainWindow::gotSMSContactInfo

void
MainWindow::contactsLinkWorkDoneSMS (bool, const QVariantList &)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (contactInfo       (const GVContactInfo &)),
         this   , SLOT   (sendTextToContact (const GVContactInfo &)));

    setStatus ("Retrieved contact info");
}//MainWindow::contactsLinkWorkDoneSMS

void
MainWindow::sendSMS (const QStringList &arrNumbers, const QString &strText)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QStringList arrFailed;

    for (int i = 0; i < arrNumbers.size (); i++)
    {
        QVariantList l;
        l += arrNumbers[i];
        l += strText;
        if (!webPage.enqueueWork (GVAW_sendSMS, l, this,
                SLOT (sendSMSDone (bool, const QVariantList &))))
        {
            arrFailed += arrNumbers[i];
            QString msg = QString ("Failed to send an SMS to %1")
                                   .arg (arrNumbers[i]);
            log (msg, 3);
            break;
        }
    } // loop through all the numbers

    if (0 != arrFailed.size ())
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
                                    "SMS could not be sent to the following:",
                                    arrFailed.join (", "),
                                    QMessageBox::Close,
                                    this);
        msgBox->setModal (false);
        QObject::connect (
            msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
            this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
        msgBox->show ();
    }
}//MainWindow::sendSMS

void
MainWindow::sendSMSDone (bool bOk, const QVariantList &)
{
    if (!bOk)
    {
        setStatus ("Failed to send SMS");
    }
    else
    {
        setStatus ("SMS sent");
    }
}//MainWindow::sendSMSDone

void
MainWindow::playVoicemail (const QString &strVmailLink)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();

    do // Begin cleanup block (not a loop)
    {
        if (mapVmail.contains (strVmailLink))
        {
            emit log ("Playing cached vmail");
            vmailPlayer.play (mapVmail[strVmailLink]);
            break;
        }

        QString strTemplate = QDir::tempPath ()
                            + QDir::separator ()
                            + "qgv_XXXXXX.tmp.mp3";
        QTemporaryFile tempFile (strTemplate);
        if (!tempFile.open ())
        {
            log ("Failed to get a temp file name");
            break;
        }
        QString strTemp = QFileInfo (tempFile.fileName ()).absoluteFilePath ();
        tempFile.close ();

        QVariantList l;
        l += strVmailLink;
        l += strTemp;
        if (!webPage.enqueueWork (GVAW_playVmail, l, this,
                SLOT (playVoicemailDone (bool, const QVariantList &))))
        {
            log ("Failed to play Voice mail", 3);
            break;
        }
    } while (0); // End cleanup block (not a loop)
}//MainWindow::playVoicemail

void
MainWindow::playVoicemailDone (bool bOk, const QVariantList &arrParams)
{
    QString strFilename = arrParams[1].toString ();
    if (bOk)
    {
        QString strVmailLink = arrParams[0].toString ();
        if (!mapVmail.contains (strVmailLink))
        {
            mapVmail[strVmailLink] = strFilename;
        }
        else
        {
            QFile::remove (strFilename);
        }

        vmailPlayer.play (mapVmail[strVmailLink]);
    }
    else
    {
        QFile::remove (strFilename);
    }
}//MainWindow::playVoicemailDone

void
MainWindow::wakeupTimedOut ()
{
    UniqueAppHelper &unique = Singletons::getRef().getUAH ();
    if (unique.isWakeSignaled ())
    {
        this->show ();
    }
    wakeupTimer.start ();
}//MainWindow::wakeupTimedOut
