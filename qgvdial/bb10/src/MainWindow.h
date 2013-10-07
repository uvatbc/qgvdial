/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

// Tabbed pane project template
#ifndef MainWindow_HPP_
#define MainWindow_HPP_

#include "IMainWindow.h"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/TextField>
#include <bb/cascades/ListView>
#include <bb/cascades/Button>
#include <bb/cascades/TabbedPane>
#include <bb/cascades/Page>
#include <bb/cascades/DropDown>
#include "bb/cascades/Option"

#include <QLocale>
#include <QTranslator>

// include JS Debugger / CS Profiler enabler
// this feature is enabled by default in the debug build only
#include <Qt/qdeclarativedebug.h>

using namespace bb::cascades;

namespace bb { namespace cascades { class Application; }}

class ContactsModel;

/*!
 * @brief Application pane object
 *
 *Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class MainWindow : public IMainWindow
{
    Q_OBJECT

public:
    MainWindow(QCoreApplication *app);
    virtual ~MainWindow() {}

    void uiRequestLoginDetails();
    void uiRequestTFALoginDetails(void*);
    void uiSetUserPass(bool editable);
    void uiRequestApplicationPassword();

    void uiLoginDone(int status, const QString &errStr);

    void onUserLogoutDone();
    void uiRefreshContacts();
    void uiRefreshInbox();

    void uiSetNewRegNumbersModel();
    void uiRefreshNumbers();

    void uiUpdateProxySettings(const ProxyInfo &info);

    void uiSetNewContactDetailsModel();
    void uiShowContactDetails(const ContactInfo &cinfo);

    void init();
    void log(QDateTime dt, int level, const QString &strLog);

protected:
    QObject *getQMLObject(const char *objectName);

protected slots:
    void onFakeInitDone();

    void onLoginBtnClicked();
    void onTfaDlgClosed();
    void onAppPwDlgClosed();

    void onUserRegSelectedOptionChanged(bb::cascades::Option *option);
    void onUserShowProxyPage();

private:
    Application *app;
    QmlDocument *qml;
    AbstractPane *root;

    TabbedPane  *mainTabbedPane;
    Page        *dialPage;
    ListView    *settingsList;
    Button      *loginButton;
    Page        *tfaDialog;
    Page        *appPwDialog;
    DropDown    *regNumberDropDown;
    Page        *contactsPage;
    ListView    *contactsList;

    //! Entirely transient two-factor authentication context
    void        *tfaCtx;
};

QCoreApplication *
createApplication(int argc, char **argv);

#endif /* MainWindow_HPP_ */
