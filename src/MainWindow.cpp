#include "MainWindow.h"
#include "GVWebPage.h"
#include "DlgSelectContactNumber.h"

#include <iostream>
using namespace std;

MainWindow::MainWindow (QWidget *parent/* = 0*/, Qt::WindowFlags f/* = 0*/):
QMainWindow(parent, f),
fLogfile(this),
tabMain(NULL),
txtLogs(NULL),
progressBar(NULL),
btnCancel(NULL),
gridMain(NULL),
icoGoogle(":/Google.png"),
icoSkype(":/Skype.png"),
icoSMS(":/SMS.png"),
icoPhone(":/Phone.png"),
pSystray(NULL),
dlgSMS (this),
vmailPlayer (this, Qt::Window),
stateMachine(this),
dbMain(QSqlDatabase::addDatabase ("QSQLITE"), this),
modelContacts(NULL)
{
    initLogging ();

#ifdef Q_WS_MAEMO_5
    this->setAttribute (Qt::WA_Maemo5StackedWindow);
    dlgSMS.setAttribute (Qt::WA_Maemo5StackedWindow);
    vmailPlayer.setAttribute (Qt::WA_Maemo5StackedWindow);
#endif

    dlgSMS.setWindowFlags (dlgSMS.windowFlags () | Qt::Window);
    vmailPlayer.hide ();

    GVWebPage::initParent (this);
    GVWebPage &webPage = GVWebPage::getRef ();

    // webPage.log -> this.log
    QObject::connect (&webPage, SIGNAL (log(const QString &, int)),
                       this   , SLOT   (log(const QString &, int)));

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
    for (QMap<QString,QString>::iterator i  = mapVmail.begin ();
                                         i != mapVmail.end ();
                                         i++)
    {
        QFile::remove (i.value ());
    }
}//MainWindow::~MainWindow

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
    GVWebPage &webPage = GVWebPage::getRef ();

    dbMain.init ();

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
    pContactsTable = new GVContactsTable (dbMain);
    tabMain->addTab (pContactsTable, "Contacts");

    // GV History
    pGVHistory = new GVHistory (dbMain);
    tabMain->addTab (pGVHistory, "History");

    // GV settings
    pGVSettings = new GVSettings (dbMain);
    tabMain->addTab (pGVSettings, "Settings");

    gridMain->addWidget (tabMain, 0,0, 1,4);
#if !NO_DBGINFO
    txtLogs = new QPlainTextEdit (this->centralWidget ());
    gridMain->addWidget (txtLogs, POS_BELOW_TAB-1,0, 1,4);
#endif

    tabMain->setTabsClosable (false);
    tabMain->setTabPosition (QTabWidget::West);

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

    // Dialing handshake
    QObject::connect (&webPage    , SIGNAL (dialInProgress ()),
                       this       , SLOT   (dialInProgress ()));
    QObject::connect ( this       , SIGNAL (dialCanFinish ()),
                      &webPage    , SLOT   (dialCanFinish ()));

    // pContactsTable.oneContact -> this.gotContact
    QObject::connect (pContactsTable,
                SIGNAL (oneContact (int, const QString &, const QString &)),
          this, SLOT   (gotContact (int, const QString &, const QString &)));
    // pContactsTable.allContacts -> this.getContactsDone
    QObject::connect (pContactsTable, SIGNAL (allContacts (bool)),
                      this          , SLOT   (getContactsDone (bool)));

    // pGVSettings.login -> this.doLogin
    QObject::connect (
        pGVSettings, SIGNAL (login   (const QString &, const QString &)),
        this       , SLOT   (doLogin (const QString &, const QString &)));
    // this.loginSuccess -> pGVSettings.loginDone
    QObject::connect (this       , SIGNAL (loginSuccess ()),
                      pGVSettings, SLOT (loginDone ()));
    // pGVSettings.logout -> this.doLogout
    QObject::connect (pGVSettings, SIGNAL (logout ()),
                      this       , SLOT   (doLogout ()));
    // pGVSettings.newUser -> this.beginGetAccountDetails
    QObject::connect (pGVSettings, SIGNAL (newUser ()),
                      this       , SLOT   (beginGetAccountDetails ()));
    // pGVSettings.regNumChanged -> this.regNumChanged
    QObject::connect (pGVSettings, SIGNAL (regNumChanged (const QString &)),
                      this       , SLOT   (regNumChanged (const QString &)));

    // pContactsTable.call -> this.call
    QObject::connect (
        pContactsTable, SIGNAL(callNameLink(const QString &, const QString &)),
        this          , SLOT  (callNameLink(const QString &, const QString &)));
    // pContactsTable.SMS -> this.SMS
    QObject::connect (
        pContactsTable,
            SIGNAL (sendSMSToNameLink (const QString &, const QString &)),
        this      ,
            SLOT   (sendSMSToNameLink (const QString &, const QString &)));

    // pGVHistory.call -> this.call
    QObject::connect (
        pGVHistory, SIGNAL (callNameLink (const QString &, const QString &)),
        this      , SLOT   (callNameLink (const QString &, const QString &)));
    QObject::connect (
        pGVHistory, SIGNAL (callLink (const QString &)),
        this      , SLOT   (callHistoryLink (const QString &)));

    // pGVHistory.SMS -> this.SMS
    QObject::connect (
        pGVHistory,
            SIGNAL (sendSMSToNameLink (const QString &, const QString &)),
        this      ,
            SLOT   (sendSMSToNameLink (const QString &, const QString &)));
    QObject::connect (
        pGVHistory, SIGNAL (playVoicemail (const QString &)),
        this      , SLOT   (playVoicemail (const QString &)));

    // pGVHistory.playVoicemail -> this.playVoicemail
    QObject::connect (
        pGVHistory, SIGNAL (sendSMSToLink (const QString &)),
        this      , SLOT   (sendSMSToLink (const QString &)));

    // dlgSMS.sendSMS -> this.sendSMS
    QObject::connect (
        &dlgSMS, SIGNAL (sendSMS (const QStringList &, const QString &)),
         this  , SLOT   (sendSMS (const QStringList &, const QString &)));

    // Logged in and logged out for all
    QObject::connect (this          , SIGNAL (loginSuccess ()),
                      pContactsTable, SLOT   (loginSuccess ()));
    QObject::connect (this          , SIGNAL (loggedOut ()),
                      pContactsTable, SLOT   (loggedOut ()));
    QObject::connect (this      , SIGNAL (loginSuccess ()),
                      pGVHistory, SLOT   (loginSuccess ()));
    QObject::connect (this      , SIGNAL (loggedOut ()),
                      pGVHistory, SLOT   (loggedOut ()));

    // Logging and setStatus
    QObject::connect (pDialer, SIGNAL (log(const QString &, int)),
                      this   , SLOT   (log(const QString &, int)));
    QObject::connect (pDialer, SIGNAL (status(const QString &, int)),
                      this   , SLOT   (setStatus(const QString &, int)));
    QObject::connect (pContactsTable, SIGNAL (log(const QString &, int)),
                      this          , SLOT   (log(const QString &, int)));
    QObject::connect (pContactsTable, SIGNAL (status(const QString &, int)),
                      this          , SLOT   (setStatus(const QString &, int)));
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
}//MainWindow::enterNotLoggedIn

void
MainWindow::aboutBlankDone (bool)
{
}//MainWindow::aboutBlankDone

void
MainWindow::doLogin (const QString &strU, const QString &strP)
{
    GVWebPage &webPage = GVWebPage::getRef ();

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        QVariantList l;
        l += strU;
        l += strP;

        log ("Beginning login");
        // webPage.workCompleted -> this.loginCompleted
        if (!webPage.enqueueWork (GVWW_login, l, this,
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
MainWindow::loginCompleted (bool bOk, const QVariantList &)
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
    GVWebPage &webPage = GVWebPage::getRef ();

    // Model init
    initContactsModel ();

    arrRegisteredNumbers.clear ();
    QVariantList l;
    QObject::connect(
        &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
         this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
    if (!webPage.enqueueWork (GVWW_getRegisteredPhones, l, this,
            SLOT (gotAllRegisteredPhones (bool, const QVariantList &))))
    {
        QObject::disconnect(
            &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
             this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
        log ("Failed to retrieve contacts!!", 3);
    }

    // Do this the first time (always)
    pGVHistory->refreshHistory ();
}//MainWindow::enterLoggedIn

void
MainWindow::beginGetAccountDetails ()
{
    pContactsTable->refreshContacts ();
}//MainWindow::beginGetAccountDetails

void
MainWindow::gotContact (int cnt, const QString &strName, const QString &strLink)
{
    dbMain.insertContact (modelContacts, cnt, strName, strLink);
}//MainWindow::gotContact

void
MainWindow::getContactsDone (bool bOk)
{
    if (!bOk)
    {
        if (NULL != modelContacts)
        {
            modelContacts->revertAll ();
        }

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
    else
    {
        setStatus ("Contacts retrieved. Saving. This will take some time...");
        modelContacts->submitAll ();
        setStatus ("Contacts committed to local database");
    }

    //TODO: Retrieve user settings
}//MainWindow::getContactsDone

void
MainWindow::gotRegisteredPhone (const GVRegisteredNumber &info)
{
    QString msg = QString("\"%1\"=\"%2\"")
                    .arg (info.strDisplayName)
                    .arg (info.strNumber);
    log (msg);
    arrRegisteredNumbers += info;
}//MainWindow::gotRegisteredPhone

void
MainWindow::gotAllRegisteredPhones (bool bOk, const QVariantList &)
{
    GVWebPage &webPage = GVWebPage::getRef ();
    QObject::disconnect(
        &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
         this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));

    if (!bOk)
    {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical,
            "Error",
            "Failed to retrieve registered phones",
            QMessageBox::Close);
        msgBox->setModal (false);
        QObject::connect (
            msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
            this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
        msgBox->show ();
        setStatus ("Failed to retrieve registered phones");
    }
    else
    {
        pGVSettings->setRegisteredNumbers (arrRegisteredNumbers);
        setStatus("GV callbacks retrieved.");
    }
}//MainWindow::gotAllRegisteredPhones

void
MainWindow::doLogout ()
{
    GVWebPage &webPage = GVWebPage::getRef ();
    QVariantList l;
    webPage.enqueueWork (GVWW_logout, l, this,
                         SLOT (logoutCompleted (bool, const QVariantList &)));
}//MainWindow::doLogout

void
MainWindow::logoutCompleted (bool, const QVariantList &)
{
    // This clears out the table and the view as well
    deinitContactsModel ();

    emit loggedOut ();

    setStatus ("Logout complete");
}//MainWindow::logoutCompleted

void
MainWindow::exitLoggedIn ()
{
}//MainWindow::exitLoggedIn

void
MainWindow::initContactsModel ()
{
    deinitContactsModel ();

    modelContacts = dbMain.newSqlTableModel ();
    pContactsTable->setModel (modelContacts);
    pContactsTable->hideColumn (1);
    modelContacts->submitAll ();
}//MainWindow::initContactsModel

void
MainWindow::deinitContactsModel ()
{
    if (NULL != pContactsTable)
    {
        pContactsTable->reset ();
    }

    if (NULL != modelContacts)
    {
        delete modelContacts;
        modelContacts = NULL;
    }
}//MainWindow::deinitContactsModel

void
MainWindow::callNameLink (const QString &strNameLink, const QString &strNumber)
{
    GVWebPage &webPage = GVWebPage::getRef ();

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        GVContactInfo info;
        info.strLink = strNameLink;
        if (dbMain.getContactInfo (info))
        {
            gotContactInfo (info, false);
            bOk = true;
            break;
        }

        QObject::connect (
            &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
             this   , SLOT   (gotContactInfo (const GVContactInfo &)));
        QVariantList l;
        l += strNameLink;
        l += strNumber;
        if (!webPage.enqueueWork (GVWW_getContactFromLink, l, this,
                SLOT (contactsLinkWorkDone (bool, const QVariantList &))))
        {
            QObject::disconnect (
                &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
                 this   , SLOT   (gotContactInfo (const GVContactInfo &)));
            log ("Getting contact info failed immediately", 3);
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk)
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
    }
}//MainWindow::callNameLink

void
MainWindow::callHistoryLink (const QString &strLink)
{
    GVWebPage &webPage = GVWebPage::getRef ();

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        GVContactInfo info;
        info.strLink = strLink;
        if (dbMain.getContactInfo (info))
        {
            gotContactInfo (info, false);
            bOk = true;
            break;
        }

        QObject::connect (
            &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
             this   , SLOT   (gotContactInfo (const GVContactInfo &)));
        QVariantList l;
        l += strLink;
        if (!webPage.enqueueWork (GVWW_getContactFromHistoryLink, l, this,
                SLOT (contactsLinkWorkDone (bool, const QVariantList &))))
        {
            QObject::disconnect (
                &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
                 this   , SLOT   (gotContactInfo (const GVContactInfo &)));
            log ("Getting contact info failed immediately", 3);
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk)
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
    }
}//MainWindow::callHistoryLink

void
MainWindow::gotContactInfo (const GVContactInfo &info, bool bCallback)
{
    if (bCallback)
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
}//MainWindow::gotContactInfo

void
MainWindow::contactsLinkWorkDone (bool, const QVariantList &)
{
    GVWebPage &webPage = GVWebPage::getRef ();
    QObject::disconnect (
        &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
         this   , SLOT   (gotContactInfo (const GVContactInfo &)));

    setStatus ("Retrieved contact info");
}//MainWindow::contactsLinkWorkDone

void
MainWindow::dialNow (const QString &strTarget)
{
    strCurrentDialed = strTarget;
    GVWebPage &webPage = GVWebPage::getRef ();
    QVariantList l;
    l += strTarget;
    if (!webPage.enqueueWork (GVWW_dialCallback, l, this,
            SLOT (dialComplete (bool, const QVariantList &))))
    {
        log ("Dialing failed instantly", 3);
    }
}//MainWindow::dialNow

void
MainWindow::dialInProgress ()
{
    bDialCancelled = false;

    QMessageBox msgBox;
    msgBox.setText (QString("Dialing %1. Abort / Cancel ends the call")
                    .arg(strCurrentDialed));
    msgBox.setStandardButtons (QMessageBox::Ok | QMessageBox::Abort);
    msgBox.setDefaultButton (QMessageBox::Ok);
    int ret = msgBox.exec ();
    if (QMessageBox::Ok == ret)
    {
        emit dialCanFinish ();
    }
    else
    {
        bDialCancelled = true;
        GVWebPage &webPage = GVWebPage::getRef ();
        webPage.cancelWork (GVWW_dialCallback);
    }
}//MainWindow::dialInProgress

void
MainWindow::dialComplete (bool bOk, const QVariantList &)
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
        setStatus (QString("Dial successful to %1.").arg(strCurrentDialed));
    }
}//MainWindow::dialComplete

void
MainWindow::regNumChanged (const QString &strNumber)
{
    log (QString ("Changing callback to %1").arg(strNumber));

    GVWebPage &webPage = GVWebPage::getRef ();
    QVariantList l;
    l += strNumber;
    if (!webPage.enqueueWork (GVWW_selectRegisteredPhone, l, this,
            SLOT (regNumChangeComplete (bool, const QVariantList &))))
    {
        log ("Failed to save the callback number instantly", 3);
    }
}//MainWindow::regNumChanged

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
        button->parent ()->deleteLater ();
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
}//MainWindow::tabChanged

void
MainWindow::sendSMSToLink (const QString &strLink)
{
    GVWebPage &webPage = GVWebPage::getRef ();

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        GVContactInfo info;
        info.strLink = strLink;
        if (dbMain.getContactInfo (info))
        {
            gotSMSContactInfo (info, false);
            bOk = true;
            break;
        }

        QObject::connect (
            &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
             this   , SLOT   (gotSMSContactInfo (const GVContactInfo &)));
        QVariantList l;
        l += strLink;
        if (!webPage.enqueueWork (GVWW_getContactFromHistoryLink, l, this,
                SLOT (contactsLinkWorkDoneSMS (bool, const QVariantList &))))
        {
            QObject::disconnect (
                &webPage, SIGNAL (contactInfo       (const GVContactInfo &)),
                 this   , SLOT   (gotSMSContactInfo (const GVContactInfo &)));
            log ("Getting contact info failed immediately", 3);
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk)
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
    }
}//MainWindow::sendSMSToLink

void
MainWindow::sendSMSToNameLink (const QString &strNameLink,
                               const QString &strNumber)
{
    GVWebPage &webPage = GVWebPage::getRef ();

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        GVContactInfo info;
        info.strLink = strNameLink;
        if (dbMain.getContactInfo (info))
        {
            gotSMSContactInfo (info, false);
            bOk = true;
            break;
        }

        QObject::connect (
            &webPage, SIGNAL (contactInfo       (const GVContactInfo &)),
             this   , SLOT   (gotSMSContactInfo (const GVContactInfo &)));
        QVariantList l;
        l += strNameLink;
        l += strNumber;
        if (!webPage.enqueueWork (GVWW_getContactFromLink, l, this,
                SLOT (contactsLinkWorkDoneSMS (bool, const QVariantList &))))
        {
            QObject::disconnect (
                &webPage, SIGNAL (contactInfo (const GVContactInfo &)),
                 this   , SLOT   (gotContactInfo (const GVContactInfo &)));
            log ("Getting contact info failed immediately", 3);
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)
    if (!bOk)
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
    }
}//MainWindow::sendSMSToNameLink

void
MainWindow::gotSMSContactInfo (const GVContactInfo &info, bool bCallback)
{
    if (bCallback)
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
    GVWebPage &webPage = GVWebPage::getRef ();
    QObject::disconnect (
        &webPage, SIGNAL (contactInfo       (const GVContactInfo &)),
         this   , SLOT   (gotSMSContactInfo (const GVContactInfo &)));

    setStatus ("Retrieved contact info");
}//MainWindow::contactsLinkWorkDoneSMS

void
MainWindow::sendSMS (const QStringList &arrNumbers, const QString &strText)
{
    GVWebPage &webPage = GVWebPage::getRef ();
    QStringList arrFailed;

    for (int i = 0; i < arrNumbers.size (); i++)
    {
        QVariantList l;
        l += arrNumbers[i];
        l += strText;
        if (!webPage.enqueueWork (GVWW_sendSMS, l, this,
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
    GVWebPage &webPage = GVWebPage::getRef ();

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
        if (!webPage.enqueueWork (GVWW_playVmail, l, this,
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
