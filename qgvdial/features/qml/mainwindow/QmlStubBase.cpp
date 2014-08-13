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

#include "QmlStubBase.h"
#include "QmlMainWindow.h"
#include "CQmlViewer.h"

QmlStubBase::QmlStubBase(QObject *parent)
: QObject(parent)
{
}//QmlStubBase::QmlStubBase

QObject *
QmlStubBase::findChild(QString name)
{
    QmlMainWindow *parent = (QmlMainWindow *) this->parent();
    if (NULL != parent) {
        return parent->getQMLObject(name.toLatin1().constData());
    } else {
        Q_WARN("No parent!");
        return NULL;
    }

    if (parent->mainPageStack->objectName () == name) {
        return parent->mainPageStack;
    } else if (parent->mainTabGroup->objectName () == name) {
        return parent->mainTabGroup;
    } else if (parent->loginExpand->objectName () == name) {
        return parent->loginExpand;
    } else if (parent->loginButton->objectName () == name) {
        return parent->loginButton;
    } else if (parent->tfaPinDlg->objectName () == name) {
        return parent->tfaPinDlg;
    } else if (parent->textUsername->objectName () == name) {
        return parent->textUsername;
    } else if (parent->textPassword->objectName () == name) {
        return parent->textPassword;
    } else if (parent->contactsPage->objectName () == name) {
        return parent->contactsPage;
    } else if (parent->inboxList->objectName () == name) {
        return parent->inboxList;
    } else if (parent->inboxSelector->objectName () == name) {
        return parent->inboxSelector;
    } else if (parent->proxySettingsPage->objectName () == name) {
        return parent->proxySettingsPage;
    } else if (parent->selectedNumberButton->objectName () == name) {
        return parent->selectedNumberButton;
    } else if (parent->regNumberSelector->objectName () == name) {
        return parent->regNumberSelector;
    } else if (parent->ciSelector->objectName () == name) {
        return parent->ciSelector;
    } else if (parent->statusBanner->objectName () == name) {
        return parent->statusBanner;
    } else if (parent->dialPage->objectName () == name) {
        return parent->dialPage;
    } else if (parent->smsPage->objectName () == name) {
        return parent->smsPage;
    } else if (parent->inboxDetails->objectName () == name) {
        return parent->inboxDetails;
    } else if (parent->etCetera->objectName () == name) {
        return parent->etCetera;
    } else if (parent->optContactsUpdate->objectName () == name) {
        return parent->optContactsUpdate;
    } else if (parent->optInboxUpdate->objectName () == name) {
        return parent->optInboxUpdate;
    } else if (parent->edContactsUpdateFreq->objectName () == name) {
        return parent->edContactsUpdateFreq;
    } else if (parent->edInboxUpdateFreq->objectName () == name) {
        return parent->edInboxUpdateFreq;
    }

    return NULL;
}//QmlStubBase::findChild

void
QmlStubBase::closeVkb()
{
    QmlMainWindow *parent = (QmlMainWindow *) this->parent();
    if (NULL == parent) {
        Q_WARN("No parent!");
        return;
    }

    QEvent event(QEvent::FocusOut);
    qApp->sendEvent(parent->m_view->rootObject(), &event);
}//QmlStubBase::closeVkb
