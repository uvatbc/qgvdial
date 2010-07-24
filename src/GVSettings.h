#ifndef __GVSETTINGS_H__
#define __GVSETTINGS_H__

#include "global.h"
#include "ChildWindowBase.h"
#include "CacheDatabase.h"
#include "CalloutInitiator.h"

class GVSettings : public ChildWindowBase
{
    Q_OBJECT

public:
    GVSettings (QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~GVSettings (void);

    bool refreshRegisteredNumbers ();
    QString getSelectedNumber ();

    bool causeLogin ();
    CalloutInitiator *getCallInitiator ();

signals:
    //! Emitted when the login button has been clicked with valid credentials
    void login (const QString &strUser, const QString &strPass);
    //! Emitted when the logout button has been clicked
    void logout ();

    //! Emitted when the logged in user is different from the user cached in DB
    void newUser ();

    //! Emitted when the user changes the currently selected number
    void regNumChanged (const QString &strNumber);

public slots:
    void loginDone (bool bOk = true);
    void logoutDone (bool bOk = true);

private slots:
    void btnLogin_clicked ();
    void cbNumbers_currentIndexChanged (int index);
    void cbCallouts_currentIndexChanged (int index);

    //! Invoked every time a new registered phone is retrieved
    void gotRegisteredPhone (const GVRegisteredNumber &info);
    //! Invoked every time a new registered phone is retrieved
    void gotAllRegisteredPhones (bool bOk, const QVariantList &arrParams);

    //! Invoked when any button on the message box is clicked
    void msgBox_buttonClicked (QAbstractButton *button);

private:
    void setRegNumWidget (bool bSave = true);

private:
    QGridLayout grid;
    QLabel      lblUser;
    QLineEdit   edUser;
    QLabel      lblPass;
    QLineEdit   edPass;

    //! The users registered numbers
    QLabel      lblNumbers;
    QComboBox   cbNumbers;
    QLabel      lblCallouts;
    QComboBox   cbCallouts;

    //! Login or logout
    QPushButton btnLogin;
    //! Is the button in login?
    bool        bBtnLogin;

    //! The users registered numbers
    GVRegisteredNumberArray arrNumbers;
};

#endif //__GVSETTINGS_H__
