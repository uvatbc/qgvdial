/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2014  Yuvraaj Kelkar

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

#include "DummyWindow.h"

DummyMainWindow::DummyMainWindow(QObject *parent)
: IMainWindow(parent)
{
}//DummyMainWindow::DummyMainWindow

void
DummyMainWindow::init()
{
    IMainWindow::init ();
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::init

void
DummyMainWindow::log(QDateTime /*dt*/, int /*level*/, const QString & /*strLog*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::log

void
DummyMainWindow::uiUpdateProxySettings(const ProxyInfo & /*info*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiUpdateProxySettings

void
DummyMainWindow::uiRequestLoginDetails()
{
    //TODO: Show the settings tab
    //TODO: Show login settings if it isn't already shown.
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiRequestLoginDetails

void
DummyMainWindow::onLoginButtonClicked()
{
    //TODO: Pull the user and password from the UI
    Q_ASSERT(0 == "Not implemented");
    //TODO: beginLogin (user, pass);
}//DummyMainWindow::onLoginButtonClicked

void
DummyMainWindow::uiRequestTFALoginDetails(void *ctx)
{
    //TODO: Show a dialog to the user to get the TFA pin
    Q_ASSERT(0 == "Not implemented");

    int pin = 0;
    if (pin == 0) {
        resumeTFAAuth (ctx, pin, true);
    } else {
        resumeTFAAuth (ctx, pin, false);
    }
}//DummyMainWindow::uiRequestTFALoginDetails

void
DummyMainWindow::uiSetUserPass(bool /*editable*/)
{
    //TODO: Set the user and password into the UI.
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiSetUserPass

void
DummyMainWindow::uiLoginDone(int /*status*/, const QString & /*errStr*/)
{
    //TODO: Inform the user that the login has failed.
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiLoginDone

void
DummyMainWindow::onUserLogoutDone()
{
    Q_DEBUG("Logout complete");
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::onUserLogoutDone

void
DummyMainWindow::uiRequestApplicationPassword()
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiRequestApplicationPassword()

void
DummyMainWindow::uiRefreshContacts(ContactsModel *, QString)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiRefreshContacts

void
DummyMainWindow::uiRefreshInbox()
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiRefreshInbox

void
DummyMainWindow::uiSetSelelctedInbox(const QString & /*selection*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiSetSelelctedInbox

void
DummyMainWindow::uiSetNewRegNumbersModel()
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiSetNewRegNumbersModel

void
DummyMainWindow::uiRefreshNumbers()
{
    Q_ASSERT(NULL != oPhones.m_numModel);
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiRefreshNumbers

void
DummyMainWindow::uiSetNewContactDetailsModel()
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiSetNewContactDetailsModel

void
DummyMainWindow::uiShowContactDetails(const ContactInfo &)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiShowContactDetails

void
DummyMainWindow::uiGetCIDetails(GVRegisteredNumber &, GVNumModel *)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiGetCIDetails

void
DummyMainWindow::uiShowStatusMessage(const QString & /*msg*/,
                                     quint64 /*millisec*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiShowStatusMessage

void
DummyMainWindow::uiClearStatusMessage()
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiClearStatusMessage

void
DummyMainWindow::uiFailedToSendMessage(const QString & /*destination*/,
                                       const QString & /*text*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiFailedToSendMessage

void
DummyMainWindow::uiEnableContactUpdateFrequency(bool /*enable*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiEnableContactUpdateFrequency

void
DummyMainWindow::uiSetContactUpdateFrequency(quint32 /*mins*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiSetContactUpdateFrequency

void
DummyMainWindow::uiEnableInboxUpdateFrequency(bool /*enable*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiEnableInboxUpdateFrequency

void
DummyMainWindow::uiSetInboxUpdateFrequency(quint32 /*mins*/)
{
    Q_ASSERT(0 == "Not implemented");
}//DummyMainWindow::uiSetInboxUpdateFrequency
