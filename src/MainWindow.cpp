#include "global.h"
#include "MainWindow.h"

#include "LoginDialog.h"
#include "DialCancelDlg.h"
#include "DlgSelectContactNumber.h"
#include "VMailDialog.h"

#include "PhoneNumberValidator.h"

#include <iostream>
using namespace std;

struct DialOutContext {
    CalloutInitiator *ci;
    DialCancelDlg *pDialDlg;
};

MainWindow::MainWindow (QWidget *parent)
: QDeclarativeView (parent)
, fLogfile (this)
, icoGoogle (":/Google.png")
, pSystray (NULL)
, pContactsView (NULL)
, pInboxView (NULL)
, pWebWidget (new WebWidget (this, Qt::Window))
#ifdef Q_WS_MAEMO_5
, infoBox (this)
#endif
, menuFile ("&File", this)
, actLogin ("Login...", this)
, actExit ("Exit", this)
, actViewWeb ("Show web view", this)
, bLoggedIn (false)
, modelRegNumber (this)
, indRegPhone (0)
, mtxDial (QMutex::Recursive)
, bCallInProgress (false)
, bDialCancelled (false)
{
#ifdef Q_WS_MAEMO_5
    QObject::connect(QApplication::desktop(), SIGNAL(resized(int)),
                     this                   , SLOT  (orientationChanged()));
#endif

    initLogging ();

    // This must be done at least once so that the initial qml is loaded.
    // Even if it is desktop, this must be done: The function takes care of
    // making it portrait for non-maemo.
    orientationChanged ();

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setDefaultWindowAttributes (this);

    pWebWidget->hide ();
    osd.setDefaultWindowAttributes (pWebWidget);

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
}//MainWindow::~MainWindow

void
MainWindow::initLogging ()
{
    // Initialize logging
    QString strLogfile = QDir::homePath ();
    QDir dirHome(strLogfile);
    if (!strLogfile.endsWith (QDir::separator ()))
    {
        strLogfile += QDir::separator ();
    }
    strLogfile += ".qgvdial";
    if (!QFileInfo(strLogfile).exists ()) {
        dirHome.mkdir (".qgvdial");
    }
    strLogfile += QDir::separator ();
    strLogfile += "qgvdial.log";
    fLogfile.setFileName (strLogfile);
    fLogfile.open (QIODevice::WriteOnly | QIODevice::Append);
}//MainWindow::initLogging

/** Log information to console and to log file
 * This function is invoked from the qDebug handler that is installed in main.
 * @param strText Text to be logged
 * @param level Log level
 */
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
    if (fLogfile.isOpen ()) {
        QTextStream streamLog(&fLogfile);
        streamLog << strLog << endl;
    }
}//MainWindow::log

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
MainWindow::setStatus(const QString &strText, int timeout /* = 0*/)
{
    qDebug () << strText;

#ifdef Q_WS_MAEMO_5
    infoBox.hide ();
    QLabel *theLabel = (QLabel *) infoBox.widget ();
    if (NULL == theLabel) {
        theLabel = new QLabel (strText, &infoBox);
        theLabel->setAlignment (Qt::AlignHCenter);
        infoBox.setWidget (theLabel);
    } else {
        theLabel->setText (strText);
    }
    infoBox.setTimeout (0 == timeout?3000:timeout);
    infoBox.show ();
#else
    if (NULL != pSystray) {
        pSystray->showMessage ("Status", strText,
                               QSystemTrayIcon::Information,
                               timeout);
    }
#endif
}//MainWindow::setStatus

/** Invoked when the QtSingleApplication sends a message
 * We have used a QtSignleApplication to ensure that there is only one instance
 * of our program running in a specific user context. When the user attempts to
 * fire up another instance of our application, the second instance communicates
 * with the first and tells it to show the main window. the 2nd instance then
 * self-terminates. The first instance gets the "show" command as a parameter to
 * this SLOT.
 * @param message The message passed by the other application.
 */
void
MainWindow::messageReceived (const QString &message)
{
    if (message == "show") {
        this->show ();
    }
}//MainWindow::messageReceived

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

    setStatus ("Initializing...");

    dbMain.init ();
    osd.initDialServer (this, SLOT (dialNow (const QString &)));
    osd.initTextServer (
        this, SLOT (sendSMS (const QStringList &, const QString &)),
        this, SLOT (onSendTextWithoutData (const QStringList &)));

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

    // Additional UI initializations:
    //@@UV: Need this for later
//    ui->edNumber->setValidator (new PhoneNumberValidator (ui->edNumber));
    actLogin.setShortcut (QKeySequence(Qt::CTRL + Qt::Key_L));
    actExit.setShortcut (QKeySequence(Qt::CTRL + Qt::Key_Q));
    actViewWeb.setShortcut (QKeySequence (Qt::CTRL + Qt::SHIFT + Qt::Key_W));
    menuFile.addAction (&actLogin);
    menuFile.addAction (&actExit);
    menuFile.addAction (&actViewWeb);
    this->addAction (&actLogin);
    this->addAction (&actExit);
    this->addAction (&actViewWeb);
    QObject::connect (&actLogin, SIGNAL (triggered()),
                       this    , SLOT   (on_action_Login_triggered()));
    QObject::connect (&actExit, SIGNAL (triggered()),
                       this   , SLOT   (on_actionE_xit_triggered()));
    QObject::connect (&actViewWeb, SIGNAL (triggered ()),
                       this      , SLOT (on_actionWeb_view_triggered ()));

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

/** Invoked to begin the login process.
 * This function begins the process to login to the GV website. Its async
 * completion routine is loginCompleted
 */
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

        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setLongWork (this, true);

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

/** SLOT: Invoked when user triggers the login/logout action
 * If it is a login action, the Login dialog box is shown.
 */
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

        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setLongWork (this, false);
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
        actLogin.setText ("Logout");
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
    QDesktopWidget *dWgt = QApplication::desktop();
    bool bLandscape = false;

    if (NULL != dWgt) {
        QRect screenGeometry = dWgt->screenGeometry();
        bLandscape = (screenGeometry.width() > screenGeometry.height());
    }
#ifndef Q_WS_MAEMO_5
    bLandscape = false;
#endif

    QDeclarativeContext *ctx = this->rootContext();

    if (this->source().toString().isEmpty ()) {
        ctx->setContextProperty ("myModel", &modelRegNumber);
        onRegPhoneSelectionChange (indRegPhone);
        this->setSource (QUrl ("qrc:/MainView.qml"));
        this->setResizeMode (QDeclarativeView::SizeRootObjectToView);

        // Call or text a number
        QGraphicsObject *gObj = this->rootObject();
        QObject::connect (gObj, SIGNAL (sigCall (QString)),
                          this, SLOT   (dialNow (QString)));
        QObject::connect (gObj, SIGNAL (sigText (QString)),
                          this, SLOT   (textANumber (QString)));
        QObject::connect (gObj, SIGNAL (sigContacts ()),
                          this, SLOT   (on_btnContacts_clicked ()));
        QObject::connect (gObj, SIGNAL (sigInbox ()),
                          this, SLOT   (on_btnHistory_clicked ()));
        QObject::connect (gObj, SIGNAL (sigSelChanged (int)),
                          this, SLOT   (onRegPhoneSelectionChange (int)));
    }
}//MainWindow::orientationChanged

void
MainWindow::doLogout ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    webPage.enqueueWork (GVAW_logout, l, this,
                         SLOT (logoutCompleted (bool, const QVariantList &)));

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, true);
}//MainWindow::doLogout

void
MainWindow::logoutCompleted (bool, const QVariantList &)
{
    // This clears out the table and the view as well
    deinitContactsWidget ();
    deinitInboxWidget ();

    arrNumbers.clear ();

    actLogin.setText ("Login...");

    bLoggedIn = false;

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
MainWindow::msgBox_buttonClicked (QAbstractButton *button)
{
    if (NULL != button->parent ())
    {
        button->parent()->deleteLater ();
    }
}//MainWindow::msgBox_buttonClicked

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

/** Convert a number and a key to more info into a structure with all the info.
 * @param strNumber The phone number
 * @param strNameLink The key to the associated information. This parameter is
 *          optional. If it is not present, then a dummy structure is created
 *          that has only the number as valid information.
 */
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

bool
MainWindow::findInfo (const QString &strNumber, GVContactInfo &info)
{
    info = GVContactInfo();

    QString strTrunc = strNumber;
    GVAccess::simplify_number (strTrunc, false);
    strTrunc.remove(' ').remove('+');

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if (!dbMain.getContactFromNumber (strNumber, info)) {
        qDebug ("Could not find info about this number. Using dummy info");
        info.strName = strNumber;
        GVContactNumber num;
        num.chType = 'O';
        num.strNumber = strNumber;
        info.arrPhones += num;
        info.selected = 0;
    } else {
        //
    }

    return (true);
}//MainWindow::findInfo

void
MainWindow::callNumber (const QString &strNumber,
                        const QString &strNameLink)
{
    GVContactInfo info;

    do { // Begin cleanup block (not a loop)
        if (!getInfoFrom (strNumber, strNameLink, info))
        {
            qWarning () << "Failed to get any info for num = " << strNumber
                        << ", link = " << strNameLink;
            break;
        }

        if (info.arrPhones.isEmpty ()) {
            qWarning ("No phones found!!");
            break;
        }

        // Check at least the first number
        QString strTest = info.arrPhones[0].strNumber;
        strTest.remove(QRegExp ("\\d*"))
               .remove(QRegExp ("\\s"))
               .remove('+')
               .remove('-');
        if (!strTest.isEmpty ()) {
            qWarning ("Cannot use numbers with special symbols or characters");
            break;
        }

        callWithContactInfo (info, false);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::callNumber

void
MainWindow::textANumber (const QString &strNumber,
                         const QString &strNameLink)
{
    GVContactInfo info;

    do { // Begin cleanup block (not a loop)
        if (!getInfoFrom (strNumber, strNameLink, info))
        {
            qWarning () << "Failed to get any info for num = " << strNumber
                        << ", link = " << strNameLink;
            return;
        }

        if (info.arrPhones.isEmpty ()) {
            qWarning ("No phones found!!");
            break;
        }

        // Check at least the first number
        QString strTest = info.arrPhones[0].strNumber;
        strTest.remove(QRegExp ("\\d*"))
               .remove(QRegExp ("\\s"))
               .remove('+')
               .remove('-');
        if (!strTest.isEmpty ()) {
            qWarning ("Cannot use numbers with special symbols or characters");
            break;
        }

        sendTextToContact (info, false);
    } while (0); // End cleanup block (not a loop)
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
        if (!bLoggedIn) {
            setStatus ("User is not logged in yet. Cannot make any calls.");
            break;
        }

        QMutexLocker locker (&mtxDial);
        if (bCallInProgress) {
            setStatus ("Another call is in progress. Please try again later");
            break;
        }

        GVRegisteredNumber gvRegNumber;
        if (!getDialSettings (bDialout, gvRegNumber, ci))
        {
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
            l += gvRegNumber.strDescription;
            l += QString (gvRegNumber.chType);
            if (!webPage.enqueueWork (GVAW_dialCallback, l, this,
                    SLOT (dialComplete (bool, const QVariantList &))))
            {
                setStatus ("Dialing failed instantly");
                break;
            }
        }

        OsDependent &osd = Singletons::getRef().getOSD ();
        osd.setLongWork (this, true);

        bCallInProgress = true;
        bDialCancelled = false;

        pDialDlg->setAttribute (Qt::WA_DeleteOnClose);
        pDialDlg->setModal (false);
        pDialDlg->doNonModal (strSelfNumber);
    } while (0); // End cleanup block (not a loop)
}//MainWindow::dialNow

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
    foreach (QString strNumber, arrNumbers) {
        if (strNumber.isEmpty ()) {
            qWarning ("Cannot text empty number");
            continue;
        }

        GVContactInfo info;

        // Get info about this number
        if (!findInfo (strNumber, info)) {
            qWarning () << "Unable to find information for " << strNumber;
            continue;
        }

        SMSEntry entry;
        entry.strName = info.strName;
        entry.sNumber = info.arrPhones[info.selected];

        dlgSMS.addSMSEntry (entry);
    }

    if (dlgSMS.isHidden ())
    {
        dlgSMS.show ();
    }
}//MainWindow::onSendTextWithoutData

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

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);
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
    QDeclarativeView::closeEvent (event);
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
                    .arg (info.strName)
                    .arg (info.strDescription);
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

    modelRegNumber.clear ();
    for (int i = 0; i < arrNumbers.size (); i++)
    {
        modelRegNumber.insertRow (arrNumbers[i].strName,
                                  arrNumbers[i].strDescription,
                                  arrNumbers[i].chType);
    }

    // Store the callouts in the same widget as the callbacks
    CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
    CalloutInitiatorList listCi = cif.getInitiators ();
    foreach (CalloutInitiator *ci, listCi) {
        modelRegNumber.insertRow (ci->name (), ci->selfNumber (), ci);
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
}//MainWindow::fillCallbackNumbers

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
            qDebug ("Invalid registered phone index");
            break;
        }

        gvRegNumber.chType = data.chType;
        gvRegNumber.strName = data.strName;
        gvRegNumber.strDescription = data.strDesc;
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
    QString disp = data.strName;
    if (RNT_Callback == data.type) {
        disp = "In : " + disp;
    } else if (RNT_Callout == data.type) {
        disp = "Out : " + disp;
    }

    QDeclarativeContext *ctx = this->rootContext();
    ctx->setContextProperty ("currentPhoneName", disp);

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setLongWork (this, false);
}//MainWindow::onRegPhoneSelectionChange
