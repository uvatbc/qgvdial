#ifndef __GVSETTINGS_H__
#define __GVSETTINGS_H__

#include "global.h"

#include <QtCore>
#include <QtGui>
#include "CacheDatabase.h"

class GVSettings : public QWidget
{
    Q_OBJECT

public:
    GVSettings(CacheDatabase   &db        ,
               QWidget         *parent = 0,
               Qt::WindowFlags  f      = 0);
    ~GVSettings(void);

    void setRegisteredNumbers (const GVRegisteredNumberArray &src);
    QString getSelectedNumber ();

    bool causeLogin ();

signals:
    //! Log emitter
    void log(const QString &strText, int level = 10);
    //! Status emitter
    void status(const QString &strText, int timeout = 2000);

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
    void updateMenu (QMenuBar *menuBar);

private slots:
    void btnLogin_clicked ();
    void cbNumbers_currentIndexChanged (int index);

private:
    QGridLayout grid;
    QLabel      lblUser;
    QLineEdit   edUser;
    QLabel      lblPass;
    QLineEdit   edPass;

    //! The users registered numbers
    QLabel      lblNumbers;
    QComboBox   cbNumbers;

    //! Login or logout
    QPushButton btnLogin;
    //! Is the button in login?
    bool        bBtnLogin;

    // Reference to the main SQLITE db
    CacheDatabase &dbMain;

    //! The users registered numbers
    GVRegisteredNumberArray arrNumbers;
};

#endif //__GVSETTINGS_H__
