#include "GVSettings.h"
#include <QMessageBox>

#define ROW_BELOW_PASS 2
#define CB_TEXT_BUILDER "%1 : %2"

GVSettings::GVSettings(CacheDatabase   &db              ,
                       QWidget         *parent  /* = 0*/,
                       Qt::WindowFlags  f       /* = 0*/)
: ChildWindowBase (parent, f)
, grid(this)
, lblUser("User name", this)
, edUser(this)
, lblPass("Password", this)
, edPass(this)
, lblNumbers("Phones", this)
, cbNumbers(this)
, btnLogin(this)
, dbMain(db)
{
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
    logoutDone (!bOk);
    if (bOk)
    {
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
        }
    }
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
        grid.addWidget (&lblNumbers, ROW_BELOW_PASS,0);
        grid.addWidget (&cbNumbers , ROW_BELOW_PASS,1);
        grid.addWidget (&btnLogin  , ROW_BELOW_PASS+1,0, 1,2);
        lblNumbers.show ();
        cbNumbers.show ();
        cbNumbers.setEnabled (false);
    }
}//GVSettings::logoutDone

void
GVSettings::setRegisteredNumbers (const GVRegisteredNumberArray &src)
{
    if (bBtnLogin)
    {
        // Means we aren't logged in. So don't do this shit
        return;
    }

    QString strCallback;
    bool bGotCallback = dbMain.getCallback (strCallback);
    int iCallback = -1;

    cbNumbers.setEnabled (true);
    arrNumbers.clear ();
    cbNumbers.clear ();
    arrNumbers = src;
    for (int i = 0; i < arrNumbers.size (); i++)
    {
        QString strText = QString (CB_TEXT_BUILDER)
                            .arg (arrNumbers[i].strDisplayName)
                            .arg (arrNumbers[i].strNumber);
        cbNumbers.addItem (strText, QVariant (i));

        if ((bGotCallback) && (strCallback == arrNumbers[i].strNumber))
        {
            iCallback = i;
        }
    }

    if ((bGotCallback) && (-1 != iCallback))
    {
        cbNumbers.setCurrentIndex (iCallback);
    }
}//GVSettings::setRegisteredNumbers

QString
GVSettings::getSelectedNumber ()
{
    return (arrNumbers[cbNumbers.currentIndex()].strNumber);
}//GVSettings::getSelectedNumber

void
GVSettings::cbNumbers_currentIndexChanged (int index)
{
    if ((index >= 0) && (arrNumbers.size () > index))
    {
        QString strCallback = arrNumbers[index].strNumber;
        dbMain.putCallback (strCallback);
        emit regNumChanged (strCallback);
    }
}//GVSettings::cbNumbers_currentIndexChanged

bool
GVSettings::setStacked ()
{
    bool rv = false;
#ifdef Q_WS_MAEMO_5
    this->setAttribute (Qt::WA_Maemo5StackedWindow);
    rv = true;
#endif
    return (rv);
}//GVSettings::setStacked
