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

#include "MainWindow.h"
#include "AbstractItemModel.hpp"
#include "ContactsModel.h"
#include "InboxModel.h"
#include "GVNumModel.h"

using namespace bb::cascades;

QCoreApplication *
createApplication(int argc, char **argv)
{
	Application *app = new Application(argc, argv);

    // localization support
    QTranslator translator;
    QString locale_string = QLocale().name();
    QString filename = QString( "qgvdial_%1" ).arg( locale_string );
    if (translator.load(filename, "app/native/qm")) {
        app->installTranslator( &translator );
    }

	return app;
}

MainWindow::MainWindow(QCoreApplication *_app)
: IMainWindow(_app)
, app ((bb::cascades::Application *) _app)
, qml(NULL)
, root(NULL)
, mainTabbedPane(NULL)
, dialPage(NULL)
, settingsList(NULL)
, loginButton(NULL)
, tfaDialog(NULL)
, appPwDialog(NULL)
, regNumberDropDown(NULL)
, contactsPage(NULL)
, contactsList(NULL)
, inboxList(NULL)
, tfaCtx(NULL)
{
}//MainWindow::MainWindow

void
MainWindow::log(QDateTime /*dt*/, int /*level*/, const QString & /*strLog*/)
{
    //TODO: Something meaningful
}//MainWindow::log

QObject *
MainWindow::getQMLObject(const char *objectName)
{
    QObject *rv = NULL;

    if (NULL == root) {
        Q_WARN("root is NULL");
        return NULL;
    }

    if (root->objectName() == objectName) {
        return root;
    }

    rv = root->findChild <QObject *> (objectName);
    if (NULL == rv) {
        Q_WARN(QString("Unable to find page %1").arg(objectName));
    }

    return (rv);
}//MainWindow::getQMLObject

void
MainWindow::init()
{
    qmlRegisterType<QAbstractItemModel>();
    qmlRegisterType<AbstractItemModel>("com.kdab.components", 1, 0,
                                       "AbstractItemModel");

        // create scene document from main.qml asset
    // set parent to created document to ensure it exists for the whole application lifetime
    qml = QmlDocument::create("asset:///main.qml").parent(this);

    // create root object for the UI
    root = qml->createRootObject<AbstractPane>();
    // set created root object as a scene
    app->setScene(root);

    IMainWindow::init ();
    QTimer::singleShot (1, this, SLOT(onFakeInitDone()));
}//MainWindow::init

void
MainWindow::onFakeInitDone()
{
    dialPage       = (Page *)       getQMLObject("DialPage");
    connect(dialPage, SIGNAL(call(QString)), this, SLOT(onUserCall(QString)));

    mainTabbedPane = (TabbedPane *) getQMLObject("MainTabbedPane");

    settingsList   = (ListView *)   getQMLObject("SettingsList");
    connect(settingsList, SIGNAL(sigShowProxy()),
            this, SLOT(onUserShowProxyPage()));

    loginButton    = (Button *)     getQMLObject("LoginButton");
    connect(loginButton, SIGNAL(clicked()), this, SLOT(onLoginBtnClicked()));

    regNumberDropDown = (DropDown *)getQMLObject("RegNumberDropDown");
    connect(regNumberDropDown, SIGNAL(selectedOptionChanged(bb::cascades::Option*)),
            this, SLOT(onUserRegSelectedOptionChanged(bb::cascades::Option*)));

    contactsPage   = (Page *)       getQMLObject("ContactsPage");

    contactsList   = (ListView *)   getQMLObject("ContactsList");
    connect(contactsList, SIGNAL(clicked(QString)),
            &oContacts, SLOT(getContactInfoAndModel(QString)));

    contactsList   = (ListView *)   getQMLObject("InboxList");
    //connect(contactsList, SIGNAL(clicked(QString)),
    //        &oInbox, SLOT(getContactInfoAndModel(QString)));

    onInitDone();
}//MainWindow::onFakeInitDone

void
MainWindow::onLoginBtnClicked()
{
    TextField *textField;
    QString user, pass;

    textField = (TextField *) getQMLObject("TextUsername");
    user = textField->text();
    textField = (TextField *) getQMLObject("TextPassword");
    pass = textField->text();

    beginLogin(user, pass);
}//MainWindow::onLoginBtnClicked

void
MainWindow::uiRequestLoginDetails()
{
    int val;
	// Open the settings page
    val = 3;
    QMetaObject::invokeMethod (mainTabbedPane, "showTab", Q_ARG(QVariant, val));
    // In the settings page, show the login details
    val = 0;
    QMetaObject::invokeMethod (settingsList, "pushMe", Q_ARG(QVariant, val));
}//MainWindow::uiRequestLoginDetails

void
MainWindow::uiRequestTFALoginDetails(void *ctx)
{
    tfaCtx = ctx;
    // Open TFA dialog
    QMetaObject::invokeMethod (settingsList, "showTfaDialog");

    tfaDialog = (Page *) getQMLObject("TFADialog");
    disconnect(tfaDialog, SIGNAL(done()), this, SLOT(onTfaDlgClosed()));
    connect(tfaDialog, SIGNAL(done()), this, SLOT(onTfaDlgClosed()));
}//MainWindow::uiRequestTFALoginDetails

void
MainWindow::onTfaDlgClosed()
{
    bool accepted = tfaDialog->property("accepted").toBool();
    int pin = tfaDialog->property("pin").toInt();
    if (accepted) {
        resumeTFAAuth(tfaCtx, pin, false);
    } else {
        resumeTFAAuth(tfaCtx, pin, true);
    }

    tfaDialog = NULL;
}//MainWindow::onTfaDlgClosed

void
MainWindow::uiSetUserPass(bool /*editable*/)
{
    TextField *textField;
    textField = (TextField *) getQMLObject("TextUsername");
    if (NULL != textField) {
        textField->setText(m_user);
    }
    textField = (TextField *) getQMLObject("TextPassword");
    if (NULL != textField) {
        textField->setText(m_pass);
    }
}//MainWindow::uiSetUserPass

void
MainWindow::uiRequestApplicationPassword()
{
    // Open app specific password dialog
    QMetaObject::invokeMethod (settingsList, "showAppPwDialog");

    appPwDialog = (Page *) getQMLObject("AppPwDialog");
    disconnect(appPwDialog, SIGNAL(done()), this, SLOT(onAppPwDlgClosed()));
    connect(appPwDialog, SIGNAL(done()), this, SLOT(onAppPwDlgClosed()));
}//MainWindow::uiRequestApplicationPassword

void
MainWindow::onAppPwDlgClosed()
{
    bool accepted = appPwDialog->property("accepted").toBool();
    QString appPw = appPwDialog->property("appPw").toString();
    if (accepted) {
        onUiGotApplicationPassword(appPw);
    }
    appPwDialog = NULL;
}//MainWindow::onAppPwDlgClosed

void
MainWindow::uiLoginDone(int status, const QString &errStr)
{
    if (ATTS_SUCCESS == status) {
        Q_DEBUG("Login successful");
        return;
    }

    QString msg;
    if (ATTS_NW_ERROR == status) {
        msg = "Network error. Try again later.";
    } else if (ATTS_USER_CANCEL == status) {
        msg = "User canceled login.";
    } else {
        msg = "Login failed";
    }
    Q_WARN(msg);

    msg += QString(" Error string:\n%1").arg(errStr);
    QMetaObject::invokeMethod (settingsList, "showAppPwDialog",
                               Q_ARG(QVariant, msg));
}//MainWindow::uiLoginDone

void
MainWindow::onUserLogoutDone()
{
    Q_DEBUG("Logout complete");
}//MainWindow::onUserLogoutDone

void
MainWindow::uiRefreshContacts()
{
    qml->setContextProperty("g_contactsModel", oContacts.m_contactsModel);
}//MainWindow::uiRefreshContacts

void
MainWindow::uiRefreshInbox()
{
    qml->setContextProperty("g_inboxModel", oInbox.m_inboxModel);
}//MainWindow::uiRefreshInbox

void
MainWindow::uiSetNewRegNumbersModel()
{
    // Absolutely nothing to do here...
}//MainWindow::uiSetNewRegNumbersModel

void
MainWindow::uiRefreshNumbers()
{
    Option *option;
    QString name;

    GVRegisteredNumber selectedNum;
    oPhones.m_numModel->getSelectedNumber(selectedNum);

    oPhones.m_ignoreSelectedNumberChanges = true;
    regNumberDropDown->removeAll();
    GVRegisteredNumberArray all = oPhones.m_numModel->getAll();
    foreach (GVRegisteredNumber num, all) {
        name = QString("%1 (%2)").arg(num.name, num.number);
        option = Option::create().text(name)
                                 .selected(num.id == selectedNum.id)
                                 .value(num.id);

        regNumberDropDown->add(option);
    }
    oPhones.m_ignoreSelectedNumberChanges = false;
}//MainWindow::uiRefreshNumbers

void
MainWindow::onUserRegSelectedOptionChanged(bb::cascades::Option *option)
{
    if (NULL == option) {
        Q_WARN("No option selected");
        return;
    }

    QString id = option->value().toString();
    if (oPhones.onUserSelectPhone(id)) {
        Q_DEBUG(QString("User selected phone with id %1").arg(id));
    }
}//MainWindow::onUserRegSelectedOptionChanged

void
MainWindow::onUserShowProxyPage()
{
    ProxyInfo info;
    if (!db.getProxyInfo (info)) {
        info.enableProxy = false;
        info.useSystemProxy = false;
        info.server.clear();
        info.port = 0;
        info.authRequired = false;
        info.user.clear();
        info.pass.clear();
    }
    QMetaObject::invokeMethod (settingsList, "showProxyPage",
                               Q_ARG(QVariant, info.enableProxy),
                               Q_ARG(QVariant, info.useSystemProxy),
                               Q_ARG(QVariant, info.server),
                               Q_ARG(QVariant, info.port),
                               Q_ARG(QVariant, info.authRequired),
                               Q_ARG(QVariant, info.user),
                               Q_ARG(QVariant, info.pass));
}//MainWindow::onUserShowProxyPage

void
MainWindow::uiUpdateProxySettings(const ProxyInfo & /*info*/)
{
    // This is not the place to set the proxy info...
}//MainWindow::uiUpdateProxySettings

void
MainWindow::uiSetNewContactDetailsModel()
{
    Q_DEBUG("Got new contact details model");
}//MainWindow::uiSetNewContactDetailsModel

void
MainWindow::uiShowContactDetails(const ContactInfo &cinfo)
{
    Q_DEBUG(QString("Show contact details for %1").arg(cinfo.strTitle));
}//MainWindow::uiShowContactDetails
