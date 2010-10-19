#include "global.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "LoginDialog.h"
#include "DialCancelDlg.h"
#include "DlgSelectContactNumber.h"
#include "VMailDialog.h"

#include "PhoneNumberValidator.h"

#include <iostream>
using namespace std;

#define CB_TEXT_BUILDER "%1 : %2"

struct DialOutContext {
    CalloutInitiator *ci;
    DialCancelDlg *pDialDlg;
};

MainWindow::MainWindow (QWidget *parent)
: QMainWindow (parent)
, ui (new Ui::MainWindow)
, icoGoogle (":/Google.png")
, pSystray (NULL)
, pContactsView (NULL)
, pInboxView (NULL)
, pWebWidget (new WebWidget (this, Qt::Window))
, bLoggedIn (false)
, mtxDial (QMutex::Recursive)
, bCallInProgress (false)
, bDialCancelled (false)
{
    ui->setupUi(this);

    // Additional UI initializations:
    ui->btnDelete->setDelete (true);
    ui->edNumber->setValidator (new PhoneNumberValidator (ui->edNumber));

    QString strDialpadFile;
    QFile fDialpad_Style;
    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setDefaultWindowAttributes (this);

#ifdef Q_WS_MAEMO_5
    QObject::connect(QApplication::desktop(), SIGNAL(resized(int)),
                     this                   , SLOT  (orientationChanged()));

    strDialpadFile = "./stylesheets/dialpad_maemo.qss";
    if (!QFileInfo(strDialpadFile).exists ()) {
        strDialpadFile = ":/dialpad_maemo.qss";
    }
#else
    strDialpadFile = "./stylesheets/dialpad_desktop.qss";
    if (!QFileInfo(strDialpadFile).exists ()) {
        strDialpadFile = ":/dialpad_desktop.qss";
    }
#endif

    pWebWidget->hide ();
    osd.setDefaultWindowAttributes (pWebWidget);

    fDialpad_Style.setFileName (strDialpadFile);
    if (fDialpad_Style.open (QIODevice::ReadOnly)) {
        ui->wgtDialpad->setStyleSheet (fDialpad_Style.readAll ());
    }

    // A systray icon if the OS supports it
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

    QObject::connect (qApp, SIGNAL (messageReceived (const QString &)),
                      this, SLOT   (messageReceived (const QString &)));

    QTimer::singleShot (1000, this, SLOT (init()));
}//MainWindow::MainWindow

MainWindow::~MainWindow ()
{
    delete ui;
}//MainWindow::~MainWindow

void
MainWindow::log (const QString &strText, int level /*= 10*/)
{
    QDateTime dt = QDateTime::currentDateTime ();
    QString strLog = QString("%1 : %2 : %3")
                     .arg(dt.toString ("yyyy-MM-dd hh:mm:ss.zzz"))
                     .arg(level)
                     .arg(strText);

    // Send to plain text widget
//    if (NULL != txtLogs) {
//        txtLogs->appendPlainText (strLog);
//    }

    // Send to standard output
    cout << strLog.toStdString () << endl;

    // Send to log file
//    if (fLogfile.isOpen ()) {
//        QTextStream streamLog(&fLogfile);
//        streamLog << strLog << endl;
//    }
}//MainWindow::log

void
MainWindow::setStatus(const QString &strText, int timeout /* = 0*/)
{
    qDebug () << strText;
    ui->statusBar->showMessage (strText, timeout);
}//MainWindow::setStatus

void
MainWindow::messageReceived (const QString &message)
{
    if (message == "show") {
        this->show ();
    }
}//MainWindow::messageReceived

void
MainWindow::init ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    OsDependent &osd = Singletons::getRef().getOSD ();

    setStatus ("Initializing...");

    dbMain.init ();
    osd.initDialServer (this, SLOT (dialNow (const QString &)));

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

    // Skype client factory main widget and status
    SkypeClientFactory &skypeFactory = Singletons::getRef().getSkypeFactory ();
    skypeFactory.setMainWidget (this);
    QObject::connect (
        &skypeFactory, SIGNAL (status(const QString &, int)),
         this        , SLOT   (setStatus(const QString &, int)));

    // Observer factory init and status
    ObserverFactory &obF = Singletons::getRef().getObserverFactory ();
    obF.init ();
    QObject::connect (&obF , SIGNAL (status(const QString &, int)),
                       this, SLOT   (setStatus(const QString &, int)));

    // webPage init and status
    QObject::connect (&webPage, SIGNAL (status(const QString &, int)),
                       this   , SLOT   (setStatus(const QString &, int)));

    // call initiator init and status
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    QObject::connect (&cif , SIGNAL (status(const QString &, int)),
                       this, SLOT   (setStatus(const QString &, int)));

    // Send an SMS
    QObject::connect (
        &dlgSMS, SIGNAL (sendSMS (const QStringList &, const QString &)),
         this  , SLOT   (sendSMS (const QStringList &, const QString &)));

    // If the cache has the username and password, begin login
    if (dbMain.getUserPass (strUser, strPass))
    {
        QVariantList l;
        logoutCompleted (true, l);
        doLogin ();
    }
    else
    {
        setStatus ("No user credentials cached. Please login");

        strUser.clear ();
        strPass.clear ();

        on_action_Login_triggered ();
    }
}//MainWindow::init

void
MainWindow::doLogin ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;

    bool bOk = false;
    do // Begin cleanup block (not a loop)
    {
        l += strUser;
        l += strPass;

        setStatus ("Logging in...", 0);
        // webPage.workCompleted -> this.loginCompleted
        if (!webPage.enqueueWork (GVAW_login, l, this,
                SLOT (loginCompleted (bool, const QVariantList &))))
        {
            qWarning ("Login returned immediately with failure!");
            break;
        }

        bOk = true;
    } while (0); // End cleanup block (not a loop)

    if (!bOk)
    {
        // Cleanup if any
        strUser.clear ();
        strPass.clear ();

        l.clear ();
        logoutCompleted (true, l);
    }
}//MainWindow::doLogin

void
MainWindow::on_action_Login_triggered ()
{
    do // Begin cleanup block (not a loop)
    {
        if (!bLoggedIn) {
            LoginDialog dlg (strUser, strPass, this);
            if (QDialog::Rejected == dlg.exec ()) {
                setStatus ("User cancelled login");
                break;
            }
            if (!dlg.getUserPass (strUser, strPass)) {
                setStatus ("Invalid username or password");
                break;
            }

            doLogin ();
        } else {
            doLogout ();
        }
    } while (0); // End cleanup block (not a loop)
}//MainWindow::on_action_Login_triggered

void
MainWindow::loginCompleted (bool bOk, const QVariantList &varList)
{
    strSelfNumber.clear ();

    if (!bOk)
    {
        setStatus ("User login failed");

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

        QVariantList l;
        logoutCompleted (true, l);
    }
    else
    {
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        setStatus ("User logged in");

        // Save the users GV number returned by the login completion
        strSelfNumber = varList[varList.size()-1].toString ();
        // Prepare then contacts widget for usage
        initContactsWidget ();
        // Prepare the inbox widget for usage
        initInboxWidget ();

        // Allow access to buttons and widgets
        ui->action_Login->setText ("Logout");
        ui->btnContacts->setEnabled (true);
        ui->btnHistory->setEnabled (true);
        ui->cbDialMethod->setEnabled (true);
        ui->btnCall->setEnabled (true);
        bLoggedIn = true;

        // Save the user name and password that was used to login
        dbMain.putUserPass (strUser, strPass);

        // Fill up the combobox on the main page
        if ((!dbMain.getRegisteredNumbers (arrNumbers)) ||
            (0 == arrNumbers.size ()))
        {
            refreshRegisteredNumbers ();
        }
        else
        {
            fillCallbackNumbers (false);
        }
    }
}//MainWindow::loginCompleted

void
MainWindow::orientationChanged ()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()) {
        ui->gridMain->removeWidget (ui->wgtDialpad);
        ui->gridMain->addWidget (ui->wgtDialpad, 0, 1);
    } else {
        ui->gridMain->removeWidget (ui->wgtDialpad);
        ui->gridMain->addWidget (ui->wgtDialpad, 1, 0);
    }
}//MainWindow::orientationChanged

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
    deinitContactsWidget ();
    deinitInboxWidget ();

    arrNumbers.clear ();

    ui->action_Login->setText ("Login...");
    ui->btnContacts->setEnabled (false);
    ui->btnHistory->setEnabled (false);
    ui->cbDialMethod->setEnabled (false);
    ui->btnCall->setEnabled (false);

    bLoggedIn = false;

    setStatus ("Logout complete");
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
MainWindow::msgBox_buttonClicked (QAbstractButton *button)
{
    if (NULL != button->parent ())
    {
        button->parent()->deleteLater ();
    }
}//MainWindow::msgBox_buttonClicked

void
MainWindow::charClicked (QChar ch)
{
    ui->edNumber->insert (ch);
}//MainWindow::charClicked

void
MainWindow::charDeleted ()
{
    ui->edNumber->backspace ();
}//MainWindow::charDeleted

void
MainWindow::on_actionE_xit_triggered ()
{
    this->close ();

    for (QMap<QString,QString>::iterator i  = mapVmail.begin ();
                                         i != mapVmail.end ();
                                         i++)
    {
        QFile::remove (i.value ());
    }
    mapVmail.clear ();

    qApp->quit ();
}//MainWindow::on_actionE_xit_triggered

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
MainWindow::initContactsWidget ()
{
    do { // Begin cleanup block (not a loop)
        if (NULL != pContactsView) {
            qDebug ("Contacts widget is already active");
            break;
        }

        // Create the contact view
        pContactsView = new GVContactsTable (this);

        // Status
        QObject::connect (
            pContactsView, SIGNAL (status   (const QString &, int)),
            this         , SLOT   (setStatus(const QString &, int)));

        // pContactsView.allContacts -> this.getContactsDone
        QObject::connect (pContactsView, SIGNAL (allContacts (bool)),
                          this         , SLOT   (getContactsDone (bool)));
        // pContactsView.call -> this.call
        QObject::connect (
            pContactsView,
                SIGNAL(callNumber(const QString &, const QString &)),
            this         ,
                SLOT  (callNumber(const QString &, const QString &)));
        // pContactsView.SMS -> this.SMS
        QObject::connect (
            pContactsView,
                SIGNAL (textANumber (const QString &, const QString &)),
            this         ,
                SLOT   (textANumber (const QString &, const QString &)));

        pContactsView->setUserPass (strUser, strPass);
        pContactsView->loginSuccess ();
        pContactsView->initModel ();

#ifndef Q_WS_MAEMO_5
        pContactsView->refreshContacts ();
#endif
    } while (0); // End cleanup block (not a loop)
}//MainWindow::initContactsWidget

void
MainWindow::deinitContactsWidget ()
{
    do { // Begin cleanup block (not a loop)
        if (NULL == pContactsView) {
            qDebug ("Contacts widget was NULL.");
            break;
        }

        pContactsView->deinitModel ();

        pContactsView->loggedOut ();

        pContactsView->deleteLater ();
        pContactsView = NULL;
    } while (0); // End cleanup block (not a loop)
}//MainWindow::deinitContactsWidget

void
MainWindow::initInboxWidget ()
{
    do { // Begin cleanup block (not a loop)
        if (NULL != pInboxView) {
            qDebug ("Inbox widget is already active");
            break;
        }

        // Create the contact view
        pInboxView = new GVHistory (this);

        // Status
        QObject::connect (
            pInboxView, SIGNAL (status   (const QString &, int)),
            this      , SLOT   (setStatus(const QString &, int)));

        // pInboxView.call -> this.call
        QObject::connect (
            pInboxView, SIGNAL(callNumber(const QString &, const QString &)),
            this      , SLOT  (callNumber(const QString &, const QString &)));
        // pInboxView.SMS -> this.SMS
        QObject::connect (
            pInboxView, SIGNAL(textANumber (const QString &, const QString &)),
            this      , SLOT  (textANumber (const QString &, const QString &)));
        // pInboxView.retrieveVoicemail -> this.retrieveVoicemail
        QObject::connect (
            pInboxView, SIGNAL(retrieveVoicemail (const QString &)),
            this      , SLOT  (retrieveVoicemail (const QString &)));

        pInboxView->loginSuccess ();
        pInboxView->initModel ();

#ifndef Q_WS_MAEMO_5
        pInboxView->refreshHistory ();
#endif
    } while (0); // End cleanup block (not a loop)
}//MainWindow::initInboxWidget

void
MainWindow::deinitInboxWidget ()
{
    do { // Begin cleanup block (not a loop)
        if (NULL == pInboxView) {
            qWarning ("Inbox widget was NULL.");
            break;
        }

        pInboxView->deinitModel ();

        pInboxView->loggedOut ();

        pInboxView->deleteLater ();
        pInboxView = NULL;
    } while (0); // End cleanup block (not a loop)
}//MainWindow::deinitInboxWidget

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
MainWindow::callWithContactInfo (const GVContactInfo &info, bool bSaveIt)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (bSaveIt)
    {
        dbMain.putContactInfo (info);
    }
    else
    {
        qDebug ("Got contact info from cached location");
    }

    DlgSelectContactNumber dlg(info, this);
    int rv = dlg.exec ();
    do // Begin cleanup block (not a loop)
    {
        if (QDialog::Accepted != rv)
        {
            qWarning ("User canceled call");
            break;
        }
        rv = dlg.getSelection ();
        if (-1 == rv)
        {
            qWarning ("Invalid selection");
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
        QMutexLocker locker (&mtxDial);
        if (bCallInProgress) {
            setStatus ("Another call is in progress. Please try again later");
            break;
        }

        GVRegisteredNumber gvRegNumber;
        if (!getDialSettings (bDialout, gvRegNumber, ci))
        {
            setStatus ("Unable to dial out because settings are not valid");
            break;
        }

        GVAccess &webPage = Singletons::getRef().getGVAccess ();
        QVariantList l;
        l += strTarget;     // The destination number is common between the two

        DialCancelDlg *pDialDlg = new DialCancelDlg (strTarget, this);
        QObject::connect (
            pDialDlg, SIGNAL (dialDlgDone    (int, const QString &)),
            this    , SLOT   (onDialDlgClose (int, const QString &)));

        if (bDialout)
        {
            DialOutContext *ctx = new DialOutContext;
            if (NULL == ctx) {
                setStatus ("Failed to dial out because of allocation problem");
                break;
            }
            ctx->ci = ci;
            ctx->pDialDlg = pDialDlg;

            l += ci->selfNumber ();
            l += QVariant::fromValue<void*>(ctx) ;
            if (!webPage.enqueueWork (GVAW_dialOut, l, this,
                    SLOT (dialComplete (bool, const QVariantList &))))
            {
                setStatus ("Dialing failed instantly");
                break;
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
                break;
            }
        }

        bCallInProgress = true;
        bDialCancelled = false;

        pDialDlg->setAttribute (Qt::WA_DeleteOnClose);
        pDialDlg->setModal (false);
        pDialDlg->doNonModal (strSelfNumber);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::dialNow

void
MainWindow::onDialDlgClose (int retval, const QString & /*strNumber*/)
{
    // Disconnecting this
    QMutexLocker locker (&mtxDial);
    if (QMessageBox::Ok == retval)
    {
        emit dialCanFinish ();
    }
    else
    {
        bDialCancelled = true;
        GVAccess &webPage = Singletons::getRef().getGVAccess ();
        webPage.cancelWork (GVAW_dialCallback);
    }
}//MainWindow::onDialDlgClose

void
MainWindow::dialInProgress (const QString & /*strNumber*/)
{
}//MainWindow::dialInProgress

void
MainWindow::dialAccessNumber (const QString  &strAccessNumber,
                              const QVariant &context        )
{
    bool bSuccess = false;
    DialOutContext *ctx = (DialOutContext *) context.value<void *>();
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

    if (NULL != ctx) {
        ctx->ci = NULL;
        if (NULL != ctx->pDialDlg) {
            if (bSuccess) {
                ctx->pDialDlg->accept ();
            } else {
                ctx->pDialDlg->reject ();
            }
        }
        free (ctx);
        ctx = NULL;
    }
}//MainWindow::dialAccessNumber


void
MainWindow::dialComplete (bool bOk, const QVariantList &params)
{
    QMutexLocker locker (&mtxDial);
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
    bCallInProgress = false;
}//MainWindow::dialComplete

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
        qDebug ("Got contact info from cached location");
    }

    DlgSelectContactNumber dlg(info, this);
    int rv = dlg.exec ();
    do // Begin cleanup block (not a loop)
    {
        if (QDialog::Accepted != rv)
        {
            qWarning ("User canceled SMS");
            break;
        }
        rv = dlg.getSelection ();
        if (-1 == rv)
        {
            qWarning ("Invalid selection");
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
}//MainWindow::sendTextToContact

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
    QString msg;

    for (int i = 0; i < arrNumbers.size (); i++)
    {
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

void
MainWindow::on_btnCall_clicked ()
{
    QString strNum = ui->edNumber->text();
    if (0 == strNum.size()) {
        setStatus ("No number entered");
        return;
    }

    dialNow (strNum);
}//MainWindow::on_btnCall_clicked

void
MainWindow::on_btnContacts_clicked ()
{
    initContactsWidget ();
    if (pContactsView->isVisible ()) {
        pContactsView->hide ();
    } else {
        pContactsView->show ();
        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setDefaultWindowAttributes (pContactsView);
    }
}//MainWindow::on_btnContacts_clicked

void
MainWindow::on_btnHistory_clicked ()
{
    initInboxWidget ();
    if (pInboxView->isVisible ()) {
        pInboxView->hide ();
    } else {
        pInboxView->show ();
        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setDefaultWindowAttributes (pInboxView);
    }
}//MainWindow::on_btnHistory_clicked

void
MainWindow::closeEvent (QCloseEvent *event)
{
    deinitContactsWidget ();
    deinitInboxWidget ();
    QMainWindow::closeEvent (event);
}//MainWindow::closeEvent

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

        ui->cbDialMethod->clear ();
        ui->cbDialMethod->setEnabled (false);
        arrNumbers.clear ();

        QVariantList l;
        QObject::connect(
            &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
             this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
        if (!webPage.enqueueWork (GVAW_getRegisteredPhones, l, this,
                SLOT (gotAllRegisteredPhones (bool, const QVariantList &))))
        {
            QObject::disconnect(
                &webPage,
                    SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
                 this   ,
                    SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
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
    QString msg = QString("\"%1\"=\"%2\"")
                    .arg (info.strDisplayName)
                    .arg (info.strNumber);
    qDebug () << msg;

    arrNumbers += info;
}//MainWindow::gotRegisteredPhone

void
MainWindow::gotAllRegisteredPhones (bool bOk, const QVariantList &)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect(
        &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
         this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));

    do { // Begin cleanup block (not a loop)
        if (!bOk)
        {
            QMessageBox *msgBox = new QMessageBox(
                    QMessageBox::Critical,
                    "Error",
                    "Failed to retrieve registered phones",
                    QMessageBox::Close);
            msgBox->setModal (false);
            QObject::connect (
                    msgBox, SIGNAL (buttonClicked (QAbstractButton *)),
                    this  , SLOT   (msgBox_buttonClicked (QAbstractButton *)));
            msgBox->show ();
            setStatus ("Failed to retrieve all registered phones");
            break;
        }

        this->fillCallbackNumbers (true);

        setStatus ("GV callbacks retrieved.");
    } while (0); // End cleanup block (not a loop)
}//MainWindow::gotAllRegisteredPhones

void
MainWindow::fillCallbackNumbers (bool bSave)
{
    // Set the correct callback
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QString strCallback;
    bool bGotCallback = dbMain.getCallback (strCallback);
    ui->cbDialMethod->clear ();
    ui->cbDialMethod->setEnabled (true);
    for (int i = 0; i < arrNumbers.size (); i++)
    {
        QString strText = QString (CB_TEXT_BUILDER)
                            .arg (arrNumbers[i].strDisplayName)
                            .arg (arrNumbers[i].strNumber);
        ui->cbDialMethod->addItem (strText);

        if ((bGotCallback) && (strCallback == arrNumbers[i].strNumber))
        {
            ui->cbDialMethod->setCurrentIndex (i);
        }
    }

    // Store the callouts in the same widget as the callbacks
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    CalloutInitiatorList listCi = cif.getInitiators ();
    foreach (CalloutInitiator *ci, listCi) {
        void * store = ci;
        QString strText = QString (CB_TEXT_BUILDER)
                            .arg (ci->name ())
                            .arg (ci->selfNumber ());
        ui->cbDialMethod->addItem (strText, QVariant::fromValue (store));
    }

    if (bSave)
    {
        // Save all callbacks into the cache
        dbMain.putRegisteredNumbers (arrNumbers);
    }
}//MainWindow::fillCallbackNumbers

bool
MainWindow::getDialSettings (bool                 &bDialout   ,
                             GVRegisteredNumber   &gvRegNumber,
                             CalloutInitiator    *&initiator  )
{
    initiator = NULL;

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        int index = ui->cbDialMethod->currentIndex ();
        if (index < arrNumbers.size ())
        {
            gvRegNumber = arrNumbers[index];
            bDialout = false;
        }
        else
        {
            QVariant var = ui->cbDialMethod->itemData (index);
            if ((!var.isValid ()) || (var.isNull ()))
            {
                qWarning ("Invalid variant in callout numbers");
                break;
            }
            initiator = (CalloutInitiator *) var.value<void *> ();
            bDialout = true;
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
    VMailDialog *dlgVmail = new VMailDialog (this);
    dlgVmail->setAttribute (Qt::WA_DeleteOnClose);
    QObject::connect (dlgVmail, SIGNAL (status (const QString &, int)),
                      this    , SLOT   (setStatus (const QString &, int)));
    dlgVmail->play (strFile);
}//MainWindow::playVmail

void
MainWindow::on_actionWeb_view_triggered ()
{
    if (pWebWidget->isVisible ()) {
        pWebWidget->hide ();
    } else {
        pWebWidget->show ();
    }
}//MainWindow::on_actionWeb_view_triggered

void
MainWindow::on_actionLogs_triggered ()
{
    //
}//MainWindow::on_actionLogs_triggered
