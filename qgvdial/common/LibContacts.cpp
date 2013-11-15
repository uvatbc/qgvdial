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

#include "Lib.h"
#include "LibContacts.h"
#include "IMainWindow.h"
#include "ContactsModel.h"
#include "ContactNumbersModel.h"

#define GOT_PHOTO_TIMEOUT (5 * 1000)  // 5 seconds

LibContacts::LibContacts(IMainWindow *parent)
: QObject(parent)
, m_contactsModel(NULL)
, m_searchedContactsModel(NULL)
, m_contactPhonesModel(NULL)
, m_mandatoryLocalPics(true) // True because QML is not GV authenticated
{
    Q_ASSERT(NULL != parent);

    connect (&api, SIGNAL(presentCaptcha(AsyncTaskToken*,QString)),
             this, SLOT(onPresentCaptcha(AsyncTaskToken*,QString)));
    connect (&api, SIGNAL(oneContact(ContactInfo)),
             this, SLOT(onOneContact(ContactInfo)));

    m_gotPhotoTimer.setSingleShot (true);
    m_gotPhotoTimer.setInterval (GOT_PHOTO_TIMEOUT);
    connect (&m_gotPhotoTimer, SIGNAL(timeout()),
             this, SLOT(refreshModel()));
}//LibContacts::LibContacts

bool
LibContacts::login(const QString &user, const QString &pass)
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    if (!token) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    connect(token, SIGNAL(completed()),
            this, SLOT(loginCompleted()));

    token->inParams["user"] = user;
    token->inParams["pass"] = pass;
    api.login (token);

    return (true);
}//LibContacts::login

void
LibContacts::loginCompleted()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (ATTS_SUCCESS == task->status) {
        Q_DEBUG("Login successful");
        win->db.setAppPass (task->inParams["pass"].toString());

        QDateTime after;
        win->db.getLatestContact (after);
        refresh (after);
    } else {
        Q_WARN("Login failed");
        win->uiRequestApplicationPassword ();
    }

    task->deleteLater ();
}//LibContacts::loginCompleted

void
LibContacts::onPresentCaptcha(AsyncTaskToken *task, const QString &captchaUrl)
{
    Q_WARN(QString("Cannot show captcha %1").arg(captchaUrl));

    task->status = ATTS_LOGIN_FAILURE;
    task->emitCompleted ();
}//LibContacts::onPresentCaptcha

bool
LibContacts::refresh(QDateTime after /* = QDateTime()*/)
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (!task) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    connect (task, SIGNAL(completed()),
             this, SLOT(onContactsFetched()));

    bool bval = true;
    task->inParams["showDeleted"] = bval;
    task->inParams["updatedMin"] = after;

    if ((bval = api.getContacts(task))) {
        IMainWindow *win = (IMainWindow *) this->parent ();
        win->db.setQuickAndDirty (true);
    }

    return (bval);
}//LibContacts::refresh

bool
LibContacts::refreshLatest()
{
    bool rv = false;
    QDateTime latest;
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (win->db.getLatestContact (latest)) {
        this->refresh (latest);
        rv = true;
    }

    return (rv);
}//LibContacts::refreshLatest

void
LibContacts::onOneContact(ContactInfo cinfo)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (cinfo.bDeleted || (0 == cinfo.arrPhones.count ())) {
        win->db.deleteContact (cinfo.strId);
    } else {
        win->db.insertContact (cinfo);
    }
}//LibContacts::onOneContact

void
LibContacts::onContactsFetched()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->db.setQuickAndDirty (false);

    if (ATTS_SUCCESS != task->status) {
        Q_WARN("Failed to update contacts");
    } else {
        ContactsModel *oldModel = m_contactsModel;
        m_contactsModel = this->createModel ();
        if (NULL != m_contactsModel) {
            if (m_mandatoryLocalPics) {
                connect(m_contactsModel, SIGNAL(noContactPhoto(QString,QString)),
                        this, SLOT(onNoContactPhoto(QString,QString)));
            }

            win->uiRefreshContacts (m_contactsModel, QString());
            if (NULL != oldModel) {
                oldModel->deleteLater ();
            }
        } else {
            m_contactsModel = oldModel;
        }
   }

    task->deleteLater ();
}//LibContacts::onContactsFetched

ContactsModel *
LibContacts::createModel(const QString &query)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    ContactsModel *model = new ContactsModel(m_mandatoryLocalPics, this);
    win->db.refreshContactsModel (model, query);
    return (model);
}//LibContacts::createModel

void
LibContacts::onNoContactPhoto(QString contactId, QString photoUrl)
{
    if (photoUrl.isEmpty ()) {
        Q_WARN("Empty photo URL");
        return;
    }

    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task to download photo");
        return;
    }

    task->inParams["id"] = contactId;
    task->inParams["href"] = photoUrl;
    connect(task, SIGNAL(completed()), this, SLOT(onGotPhoto()));

    if (!api.getPhotoFromLink (task)) {
        Q_WARN("Unable to get photo");
        delete task;
    }

    m_gotPhotoTimer.stop ();
}//LibContacts::onNoContactPhoto

void
LibContacts::onGotPhoto()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    IMainWindow *win = (IMainWindow *) this->parent ();
    QString id   = task->inParams["id"].toString();
    QString href = task->inParams["href"].toString();

    do {
        if (ATTS_SUCCESS != task->status) {
            Q_WARN(QString("Failed to get photo for ID %1").arg (id));
            win->db.putTempFile (href, UNKNOWN_CONTACT_QRC_PATH);
            break;
        }

        Lib &lib = Lib::ref();
        QString tempPath = lib.getTempDir () + QDir::separator()
                         + tr("qgv_XXXXXX.tmp.");

        switch (task->outParams["type"].toInt()) {
        case GCPT_PNG:
            tempPath += "png";
            break;
        case GCPT_BMP:
            tempPath += "bmp";
            break;
        case GCPT_JPEG:
        default:
            tempPath += "jpg";
            break;
        }

        QTemporaryFile tempFile (tempPath);
        if (!tempFile.open ()) {
            Q_WARN("Failed to get a temp file name for the photo");
            break;
        }

        tempFile.setAutoRemove (false);
        tempFile.write (task->outParams["data"].toByteArray());

        win->db.putTempFile (href, tempFile.fileName ());
    } while(0);
    task->deleteLater ();

    m_gotPhotoTimer.stop ();
    m_gotPhotoTimer.start ();
}//LibContacts::onGotPhoto

void
LibContacts::refreshModel()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->db.refreshContactsModel (m_contactsModel);
}//LibContacts::refreshModel

bool
LibContacts::getContactInfoAndModel(ContactInfo &cinfo)
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (!win->db.getContactFromLink (cinfo)) {
        Q_WARN(QString("Couldn't find contact with ID %1").arg (cinfo.strId));
        return (false);
    }

    if (cinfo.strPhotoPath.isEmpty ()) {
        cinfo.strPhotoPath = UNKNOWN_CONTACT_QRC_PATH;
    }

    return true;
}//LibContacts::getContactInfoAndModel

bool
LibContacts::getContactInfoAndModel(QString id)
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    ContactInfo cinfo;
    cinfo.strId = id;
    if (!getContactInfoAndModel (cinfo)) {
        Q_WARN(QString("Couldn't find contact with ID %1").arg (id));
        return (false);
    }

    if (NULL == m_contactPhonesModel) {
        m_contactPhonesModel = new ContactNumbersModel(this);
        if (NULL == m_contactPhonesModel) {
            Q_WARN("Failed to create contact numbers model");
            return (false);
        }

        win->uiSetNewContactDetailsModel ();
    }
    m_contactPhonesModel->setPhones (cinfo);

    win->uiShowContactDetails (cinfo);

    return (true);
}//LibContacts::getContactInfoAndModel

bool
LibContacts::searchContacts(const QString &query)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    ContactsModel *oldModel = m_searchedContactsModel;

    if (query.isEmpty ()) {
        if (NULL == m_contactsModel) {
            m_contactsModel = this->createModel ();
            if (NULL == m_contactsModel) {
                Q_WARN("Unable to allocate contacts model");
                return false;
            }

            if (m_mandatoryLocalPics) {
                connect(m_contactsModel,SIGNAL(noContactPhoto(QString,QString)),
                        this, SLOT(onNoContactPhoto(QString,QString)));
            }
        }

        win->uiRefreshContacts (m_contactsModel, QString());

        if (NULL != oldModel) {
            oldModel->deleteLater ();
            m_searchedContactsModel = NULL;
        }
        return true;
    }

    m_searchedContactsModel = this->createModel (query);
    if (NULL == m_searchedContactsModel) {
        m_searchedContactsModel = oldModel;
        Q_WARN("Unable to allocate contacts model");
        return false;
    }

    if (m_mandatoryLocalPics) {
        connect(m_contactsModel, SIGNAL(noContactPhoto(QString,QString)),
                this, SLOT(onNoContactPhoto(QString,QString)));
    }

    win->uiRefreshContacts(m_searchedContactsModel, query);

    if (NULL != oldModel) {
        oldModel->deleteLater ();
    }

    return true;
}//LibContacts::searchContacts
