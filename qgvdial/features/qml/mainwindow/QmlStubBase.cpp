#include "QmlStubBase.h"
#include "QmlMainWindow.h"

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
