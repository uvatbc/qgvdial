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

#include "Lib.h"
#include "LibContacts.h"
#include "IMainWindow.h"
#include "ContactsModel.h"
#include "ContactNumbersModel.h"
#include "O2ContactsStore.h"

#define GOT_PHOTO_TIMEOUT (1 * 1000)  // 1 second

LibContacts::LibContacts(IMainWindow *parent)
: QObject(parent)
, m_enableTimerUpdate(false)
, m_photoMutex(QMutex::Recursive)
, m_simutaneousPhotoDownloads(0)
, m_isFirstRefresh(true)
, m_contactsModel(NULL)
, m_searchedContactsModel(NULL)
, m_contactPhonesModel(NULL)
{
    Q_ASSERT(NULL != parent);

    connect(&api, SIGNAL(presentCaptcha(AsyncTaskToken*,QString)),
            this, SLOT(onPresentCaptcha(AsyncTaskToken*,QString)));
    connect(&api, SIGNAL(oneContact(ContactInfo)),
            this, SLOT(onOneContact(ContactInfo)));
    connect(&api, SIGNAL(openBrowser(QUrl)),
            this, SLOT(onOpenBrowser(QUrl)));
    connect(&api, SIGNAL(closeBrowser()),
            this, SLOT(onCloseBrowser()));

    m_gotPhotoTimer.setSingleShot (true);
    m_gotPhotoTimer.setInterval (GOT_PHOTO_TIMEOUT);
    connect (&m_gotPhotoTimer, SIGNAL(timeout()),
             this, SLOT(refreshModel()));

    m_updateTimer.setSingleShot (true);
    connect(&m_updateTimer, SIGNAL(timeout()),
            this, SLOT(refreshLatest()));
}//LibContacts::LibContacts

void
LibContacts::init()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    api.initStore (win->db.createContactsStore ());
}//LibContacts::init

bool
LibContacts::login(const QString &user)
{
    AsyncTaskToken *token = new AsyncTaskToken(this);
    if (!token) {
        Q_WARN("Failed to allocate token");
        return false;
    }

    Q_DEBUG("Starting contacts login");

    connect(token, SIGNAL(completed()),
            this, SLOT(loginCompleted()));

    token->inParams["user"] = user;
    api.login (token);

    return (true);
}//LibContacts::login

void
LibContacts::logout()
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (!task) {
        Q_WARN("Failed to allocate token");
        return;
    }

    connect(task, SIGNAL(completed()), task, SLOT(deleteLater()));
    api.logout (task);
}//LibContacts::logout

void
LibContacts::onOpenBrowser(const QUrl &url)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->uiOpenBrowser (url);
}//LibContacts::onOpenBrowser

void
LibContacts::onCloseBrowser()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->uiCloseBrowser ();
}//LibContacts::onCloseBrowser

void
LibContacts::loginCompleted()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (ATTS_SUCCESS == task->status) {
        Q_DEBUG("Contacts login successful");
        QDateTime after;
        win->db.getLatestContact (after);
        refresh (after);
    } else {
        Q_WARN("Contacts login failed.");
        win->uiShowMessageBox ("Contacts login has failed.\n"
                               "Please restart the application");
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

    IMainWindow *win = (IMainWindow *) this->parent ();
    bool bval = true;
    task->inParams["showDeleted"] = bval;
    task->inParams["updatedMin"] = after;

    if (!after.isValid ()) {
        // This is a full refresh, so clear out the stale links
        quint32 del = win->db.clearTempFileByFile (UNKNOWN_CONTACT_QRC_PATH);
        if (0 != del) {
            Q_DEBUG(QString("Deleted %1 links all pointing to the unknown qrc "
                            "path") .arg (del));
        }
    }

    win->db.setQuickAndDirty (true);
    bval = api.getContacts(task);
    if (!bval) {
        win->db.setQuickAndDirty (false);
    }

    return (bval);
}//LibContacts::refresh

bool
LibContacts::refreshLatest()
{
    QDateTime latest;
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (!win->db.getLatestContact (latest)) {
        return (false);
    }

    win->showStatusMessage ("Starting contacts refresh", SHOW_3SEC);
    return (refresh (latest));
}//LibContacts::refreshLatest

bool
LibContacts::refreshFull()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    win->showStatusMessage ("Starting FULL contacts refresh", SHOW_3SEC);
    return refresh();
}//LibContacts::refreshFull

void
LibContacts::onOneContact(ContactInfo cinfo)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    if (cinfo.bDeleted || (0 == cinfo.arrPhones.count ())) {
        win->db.deleteContact (cinfo.strId);
    } else {
        win->db.insertContact (cinfo);

        if (!cinfo.hrefPhoto.isEmpty ()) {
            PhotoLink l;
            l.id = cinfo.strId;
            l.href = cinfo.hrefPhoto;

            QMutexLocker locker(&m_photoMutex);
            m_noPhotos.append (l);
        }
    }
}//LibContacts::onOneContact

void
LibContacts::afterFirstRefresh()
{
    if (!m_isFirstRefresh) return;
    m_isFirstRefresh = false;

    QString contactId, photoUrl, localPath;
    int count = 0;
    int rowCount = m_contactsModel->rowCount ();
    for (int i = 0; i  < rowCount; i++) {
        QSqlRecord rec = m_contactsModel->record (i);
        contactId = rec.field(0).value().toString();
        photoUrl  = rec.field(2).value().toString();
        localPath = rec.field(3).value().toString();

        if (localPath.isEmpty() ||
            ((localPath != UNKNOWN_CONTACT_QRC_PATH) && !QFileInfo(localPath).exists()))
        {
            // Local path is empty and image path is not empty.
            onNoContactPhoto (contactId, photoUrl);
            count++;
        }
    }

    Q_DEBUG(QString("Pre-fetched %1 NULL photo links").arg (count));
}//LibContacts::afterFirstRefresh

void
LibContacts::onContactsFetched()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();
    task->deleteLater ();

    IMainWindow *win = (IMainWindow *) this->parent ();
    win->db.setQuickAndDirty (false);

    if (ATTS_SUCCESS != task->status) {
        Q_WARN("Failed to update contacts");

        startNextPhoto ();

        MixPanelEvent mixEvent;
        mixEvent.distinct_id = win->m_user;
        mixEvent.event = "Contacts refresh failed";
        win->m_mixPanel.addEvent(mixEvent);
    } else {
        Q_DEBUG("Contacts updated.");

        ContactsModel *oldModel = m_contactsModel;
        m_contactsModel = this->createModel ();
        if (NULL != m_contactsModel) {
            connect(m_contactsModel,SIGNAL(noContactPhoto(QString,QString)),
                    this, SLOT(onNoContactPhoto(QString,QString)));

            win->uiRefreshContacts (m_contactsModel, QString());
            if (NULL != oldModel) {
                oldModel->deleteLater ();
            }
        } else {
            m_contactsModel = oldModel;
        }

        afterFirstRefresh ();

        startNextPhoto ();
        win->showStatusMessage ("Contacts fetched", SHOW_3SEC);
    }

    if (m_enableTimerUpdate && (m_updateTimer.interval () >= 60)) {
        m_updateTimer.stop ();
        m_updateTimer.start ();
        Q_DEBUG("Restarting update timer");
    }
}//LibContacts::onContactsFetched

ContactsModel *
LibContacts::createModel(const QString &query)
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    ContactsModel *model = new ContactsModel(this);
    win->db.refreshContactsModel (model, query);
    return (model);
}//LibContacts::createModel

void
LibContacts::startNextPhoto()
{
    IMainWindow *win = (IMainWindow *) this->parent ();
    QMutexLocker locker(&m_photoMutex);

    int c = m_noPhotos.count() + m_simutaneousPhotoDownloads;
    win->showStatusMessage (QString("%1 contact photo%2 to be fetched")
                              .arg(c).arg(c == 1 ? "" : "s"), SHOW_3SEC);

    if (m_simutaneousPhotoDownloads > 5) {
        return;
    }

    bool rv = false;
    do {
        if (m_noPhotos.isEmpty()) {
            win->showStatusMessage ("Contact photos fetched", SHOW_3SEC);
            m_gotPhotoTimer.stop ();
            m_gotPhotoTimer.start ();
            break;
        }

        PhotoLink l = m_noPhotos.takeFirst ();
        rv = getOnePhoto (l.id, l.href);
    } while(!rv);

    m_simutaneousPhotoDownloads += (rv ? 1 : 0);

    if (0 == m_simutaneousPhotoDownloads) {
        Q_ASSERT(m_noPhotos.isEmpty ());
    }
}//LibContacts::startNextPhoto

bool
LibContacts::getOnePhoto(QString contactId, QString photoUrl)
{
    if (photoUrl.isEmpty ()) {
        Q_WARN("Empty photo URL");
        return false;
    }

    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task to download photo");
        return false;
    }

    task->inParams["id"] = contactId;
    task->inParams["href"] = photoUrl;
    connect(task, SIGNAL(completed()), this, SLOT(onGotPhoto()));

    if (!api.getPhotoFromLink (task)) {
        Q_WARN("Unable to get photo");
        delete task;
        return false;
    }

    // Dont want the timer running when I start downloading. When the download
    // completes (success or failure), then I want to start it.
    m_gotPhotoTimer.stop ();
    return true;
}//LibContacts::getOnePhoto

void
LibContacts::onNoContactPhoto(QString contactId, QString photoUrl)
{
    PhotoLink l;
    l.id = contactId;
    l.href = photoUrl;

    QMutexLocker locker(&m_photoMutex);
    m_noPhotos.append (l);
    startNextPhoto ();
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
            win->db.putTempFile (href, UNKNOWN_CONTACT_QRC_PATH);
            break;
        }

        tempFile.setAutoRemove (false);
        tempFile.write (task->outParams["data"].toByteArray());

        win->db.putTempFile (href, tempFile.fileName ());
    } while(0);

    task->deleteLater ();

    QMutexLocker locker(&m_photoMutex);
    m_simutaneousPhotoDownloads--;
    startNextPhoto ();
}//LibContacts::onGotPhoto

void
LibContacts::refreshModel()
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (NULL != m_contactsModel) {
        win->db.refreshContactsModel (m_contactsModel);
    }
    if (NULL != m_searchedContactsModel) {
        Q_ASSERT(!m_searchQuery.isEmpty ());
        win->db.refreshContactsModel (m_searchedContactsModel, m_searchQuery);
    }

    Q_DEBUG("Contacts model refreshed");
}//LibContacts::refreshModel

bool
LibContacts::getContactInfoFromLink(ContactInfo &cinfo)
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    if (!win->db.getContactFromLink (cinfo)) {
        Q_WARN(QString("Couldn't find contact with ID %1").arg (cinfo.strId));
        cinfo.strPhotoPath = UNKNOWN_CONTACT_QRC_PATH;
        return (false);
    }

    if (cinfo.strPhotoPath.isEmpty ()) {
        cinfo.strPhotoPath = UNKNOWN_CONTACT_QRC_PATH;
    }

    return true;
}//LibContacts::getContactInfoFromLink

bool
LibContacts::getContactInfoFromNumber(QString num, ContactInfo &cinfo)
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    num.remove(' ').remove('+');
    if (!win->db.getContactFromNumber (num, cinfo)) {
        Q_WARN(QString("Couldn't find contact with number %1").arg (num));
        cinfo.strPhotoPath = UNKNOWN_CONTACT_QRC_PATH;
        return (false);
    }

    if (cinfo.strPhotoPath.isEmpty ()) {
        cinfo.strPhotoPath = UNKNOWN_CONTACT_QRC_PATH;
    }

    return true;
}//LibContacts::getContactInfoFromNumber

bool
LibContacts::getContactInfoAndModel(QString id)
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    ContactInfo cinfo;
    cinfo.strId = id;
    if (!getContactInfoFromLink (cinfo)) {
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

            connect(m_contactsModel,SIGNAL(noContactPhoto(QString,QString)),
                    this, SLOT(onNoContactPhoto(QString,QString)));
        }

        win->uiRefreshContacts (m_contactsModel, QString());

        if (NULL != oldModel) {
            oldModel->deleteLater ();
            m_searchedContactsModel = NULL;
            m_searchQuery.clear ();
        }
        return true;
    }

    m_searchedContactsModel = this->createModel (query);
    if (NULL == m_searchedContactsModel) {
        m_searchedContactsModel = oldModel;
        Q_WARN("Unable to allocate contacts model");
        return false;
    }
    m_searchQuery = query;
    connect(m_searchedContactsModel, SIGNAL(noContactPhoto(QString,QString)),
            this, SLOT(onNoContactPhoto(QString,QString)));
    connect(m_contactsModel, SIGNAL(noContactPhoto(QString,QString)),
            this, SLOT(onNoContactPhoto(QString,QString)));

    win->uiRefreshContacts(m_searchedContactsModel, query);

    if (NULL != oldModel) {
        oldModel->deleteLater ();
    }

    return true;
}//LibContacts::searchContacts

void
LibContacts::enableUpdateFrequency(bool enable)
{
    IMainWindow *win = (IMainWindow *) this->parent ();

    m_enableTimerUpdate = enable;

    m_updateTimer.stop ();
    if (enable) {
        quint32 mins = m_updateTimer.interval () / (1000 * 60);
        if (mins > 0) {
            Q_DEBUG(QString("Enable update at %1 minute intervals").arg(mins));
            m_updateTimer.start ();
            win->db.setContactsUpdateFreq (mins);
        } else {
            Q_DEBUG("Enable update, but no interval set");
        }
    } else {
        Q_DEBUG("Update disabled");
        win->db.clearContactsUpdateFreq ();
    }
}//LibContacts::enableUpdateFrequency

void
LibContacts::setUpdateFrequency(quint32 mins)
{
    if (mins == 0) {
        Q_WARN("Cannot set update frequency to zero minutes");
        return;
    }

    IMainWindow *win = (IMainWindow *) this->parent ();
    m_updateTimer.stop ();
    m_updateTimer.setInterval (mins * 60 * 1000);

    if (m_enableTimerUpdate) {
        win->db.setContactsUpdateFreq (mins);
        m_updateTimer.start ();
    }

    Q_DEBUG(QString("Update at %1 minute intervals set%2")
            .arg(mins).arg(m_enableTimerUpdate?"":" but not started"));
}//LibContacts::setUpdateFrequency
