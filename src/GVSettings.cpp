#include "GVSettings.h"
#include "Singletons.h"

#define ROW_BELOW_PASS 2
#define CB_TEXT_BUILDER "%1 : %2"

GVSettings::GVSettings(QWidget *parent /* = 0*/, Qt::WindowFlags f /* = 0*/)
: ChildWindowBase (parent, f)
, grid(this)
, lblUser("User name", this)
, edUser(this)
, lblPass("Password", this)
, edPass(this)
, lblNumbers("Phones", this)
, cbNumbers(this)
, lblCallouts ("Callouts", this)
, cbCallouts(this)
, btnLogin(this)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    edPass.setEchoMode (QLineEdit::Password);
    btnLogin.setText ("Login");
    bBtnLogin = true;

    QString strUser, strPass;
    if (dbMain.getUserPass (strUser, strPass))
    {
        edUser.setText (strUser);
        edPass.setText (strPass);
    }

    lblNumbers.hide ();
    cbNumbers.hide ();
    lblCallouts.hide ();
    cbCallouts.hide ();

    grid.addWidget (&lblUser , 0,0);
    grid.addWidget (&edUser  , 0,1);
    grid.addWidget (&lblPass , 1,0);
    grid.addWidget (&edPass  , 1,1);
    grid.addWidget (&btnLogin, ROW_BELOW_PASS,0, 1,2);
    this->setLayout (&grid);

    // btnLogin.clicked -> this.btnLogin_clicked
    QObject::connect (&btnLogin, SIGNAL (clicked ()),
                       this    , SLOT   (btnLogin_clicked ()));
    // cbNumbers.currentIndexChanged -> this.cbNumbers_currentIndexChanged
    QObject::connect (&cbNumbers, SIGNAL (currentIndexChanged (int)),
                       this     , SLOT   (cbNumbers_currentIndexChanged (int)));
    // cbCallouts.currentIndexChanged -> this.cbCallouts_currentIndexChanged
    QObject::connect (
        &cbCallouts, SIGNAL (currentIndexChanged (int)),
         this      , SLOT   (cbCallouts_currentIndexChanged (int)));
}//GVSettings::GVSettings

GVSettings::~GVSettings(void)
{
}//GVSettings::~GVSettings

bool
GVSettings::causeLogin ()
{
    if ((0 == edUser.text().size ()) ||
        (0 == edPass.text().size ()))
    {
        return (false);
    }

    btnLogin.setText ("Logging in...");
    btnLogin.setEnabled (false);
    cbNumbers.setEnabled (false);
    cbCallouts.setEnabled (false);

    emit status ("Logging in...");
    emit login (edUser.text (), edPass.text ());

    return (true);
}//GVSettings::causeLogin

void
GVSettings::btnLogin_clicked ()
{
    if (bBtnLogin)
    {
        if (!causeLogin ())
        {
            QMessageBox msgBox(QMessageBox::Critical,
                               "Invalid username or password",
                               "Username or password MUST not be blank",
                               QMessageBox::Close,
                               this);
            msgBox.exec ();
        }
    }
    else
    {
        btnLogin.setText ("Logging out...");
        btnLogin.setEnabled (false);

        emit status ("Logging out...");
        emit logout ();
    }
}//GVSettings::btnLogin_clicked

void
GVSettings::loginDone (bool bOk/* = true*/)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    do { // Begin cleanup block (not a loop)
        logoutDone (!bOk);
        if (!bOk)
        {
            break;
        }

        bool bSameUser = false;
        QString strUser, strPass;
        if (dbMain.getUserPass (strUser, strPass))
        {
            if (0 == edUser.text().compare (strUser, Qt::CaseInsensitive))
            {
                bSameUser = true;
            }
        }

        emit status ("Login successful!");
        dbMain.putUserPass (edUser.text (), edPass.text ());

        if (!bSameUser)
        {
            emit newUser ();
            break;
        }

        if ((!dbMain.getRegisteredNumbers (arrNumbers)) ||
            (0 == arrNumbers.size ()))
        {
            refreshRegisteredNumbers ();
            break;
        }

        this->setRegNumWidget (false);
    } while (0); // End cleanup block (not a loop)
}//GVSettings::loginDone

void
GVSettings::logoutDone (bool bOk/* = true*/)
{
    if (bOk)
    {
        bBtnLogin = true;

        arrNumbers.clear ();

        edUser.setEnabled (true);
        edPass.setEnabled (true);

        btnLogin.setText ("Login...");
        btnLogin.setEnabled (true);

        lblNumbers.hide ();
        cbNumbers.hide ();
        lblCallouts.hide ();
        cbCallouts.hide ();
        grid.removeWidget (&cbCallouts);
        grid.removeWidget (&lblCallouts);
        grid.removeWidget (&cbNumbers);
        grid.removeWidget (&lblNumbers);
        grid.removeWidget (&btnLogin);
        grid.addWidget (&btnLogin, ROW_BELOW_PASS,0, 1,2);

        emit status ("User is logged out.", 2000);
    }
    else
    {
        bBtnLogin = false;

        edUser.setEnabled (false);
        edPass.setEnabled (false);

        btnLogin.setText ("Logout...");
        btnLogin.setEnabled (true);

        grid.removeWidget (&btnLogin);
        grid.addWidget (&lblNumbers , ROW_BELOW_PASS  , 0);
        grid.addWidget (&cbNumbers  , ROW_BELOW_PASS  , 1);
        grid.addWidget (&lblCallouts, ROW_BELOW_PASS+1, 0);
        grid.addWidget (&cbCallouts , ROW_BELOW_PASS+1, 1);
        grid.addWidget (&btnLogin   , ROW_BELOW_PASS+2, 0, 1,2);
        lblNumbers.show ();
        cbNumbers.show ();
        cbNumbers.setEnabled (false);
        lblCallouts.show ();

        // Fill up cbCallouts
        CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
        CalloutInitiatorList listCi = cif.getInitiators ();
        foreach (CalloutInitiator *ci, listCi) {
            void * store = ci;
            cbCallouts.addItem (ci->name (), QVariant::fromValue (store));
        }
        cbCallouts.show ();
        cbCallouts.setEnabled (true);
    }
}//GVSettings::logoutDone

bool
GVSettings::refreshRegisteredNumbers ()
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();

    bool rv = false;
    do { // Begin cleanup block (not a loop)
        if (bBtnLogin)
        {
            // Means we aren't logged in. So don't do this shit
            break;
        }

        cbNumbers.setEnabled (false);
        arrNumbers.clear ();
        cbNumbers.clear ();
        cbCallouts.clear ();

        QVariantList l;
        QObject::connect(
            &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
             this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
        if (!webPage.enqueueWork (GVAW_getRegisteredPhones, l, this,
                SLOT (gotAllRegisteredPhones (bool, const QVariantList &))))
        {
            QObject::disconnect(
                &webPage, SIGNAL (registeredPhone    (const GVRegisteredNumber &)),
                 this   , SLOT   (gotRegisteredPhone (const GVRegisteredNumber &)));
            emit log ("Failed to retrieve registered contacts!!", 3);
            break;
        }

        // Fill up cbCallouts
        CallInitiatorFactory& cif = Singletons::getRef().getCIFactory ();
        CalloutInitiatorList listCi = cif.getInitiators ();
        foreach (CalloutInitiator *ci, listCi) {
            void * store = ci;
            cbCallouts.addItem (ci->name (), QVariant::fromValue (store));
        }

        rv = true;
    } while (0); // End cleanup block (not a loop)

    return (rv);
}//GVSettings::refreshRegisteredNumbers

void
GVSettings::gotRegisteredPhone (const GVRegisteredNumber &info)
{
    QString msg = QString("\"%1\"=\"%2\"")
                    .arg (info.strDisplayName)
                    .arg (info.strNumber);
    emit log (msg);

    arrNumbers += info;
}//GVSettings::gotRegisteredPhone

void
GVSettings::gotAllRegisteredPhones (bool bOk, const QVariantList &)
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
            emit status ("Failed to retrieve all registered phones");
            break;
        }

        this->setRegNumWidget (true);

        emit status ("GV callbacks retrieved.");
    } while (0); // End cleanup block (not a loop)
}//GVSettings::gotAllRegisteredPhones

void
GVSettings::setRegNumWidget (bool bSave)
{
    // Set the correct callback
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QString strCallback;
    bool bGotCallback = dbMain.getCallback (strCallback);
    cbNumbers.clear ();
    cbNumbers.setEnabled (true);
    for (int i = 0; i < arrNumbers.size (); i++)
    {
        QString strText = QString (CB_TEXT_BUILDER)
                            .arg (arrNumbers[i].strDisplayName)
                            .arg (arrNumbers[i].strNumber);
        cbNumbers.addItem (strText, QVariant (i));

        if ((bGotCallback) && (strCallback == arrNumbers[i].strNumber))
        {
            cbNumbers.setCurrentIndex (i);
        }
    }

    if (bSave)
    {
        // Save all callbacks into the cache
        dbMain.putRegisteredNumbers (arrNumbers);
    }
}//GVSettings::setRegNumWidget

QString
GVSettings::getSelectedNumber ()
{
    return (arrNumbers[cbNumbers.currentIndex()].strNumber);
}//GVSettings::getSelectedNumber

void
GVSettings::cbNumbers_currentIndexChanged (int index)
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    if ((index >= 0) && (arrNumbers.size () > index))
    {
        QString strCallback = arrNumbers[index].strNumber;
        dbMain.putCallback (strCallback);
        emit regNumChanged (strCallback);
    }
}//GVSettings::cbNumbers_currentIndexChanged

void
GVSettings::msgBox_buttonClicked (QAbstractButton *button)
{
    if (NULL != button->parent ())
    {
        button->parent()->deleteLater ();
    }
}//GVSettings::msgBox_buttonClicked

void
GVSettings::cbCallouts_currentIndexChanged (int index)
{
    // Is there anything to be done here?
}//GVSettings::cbCallouts_currentIndexChanged

CalloutInitiator *
GVSettings::getCallInitiator ()
{
    cbCallouts.currentIndex ();
    QVariant var = cbCallouts.itemData (cbCallouts.currentIndex ());
    CalloutInitiator *ci = (CalloutInitiator *) var.value<void *> ();

    return (ci);
}//GVSettings::getCallInitiator
