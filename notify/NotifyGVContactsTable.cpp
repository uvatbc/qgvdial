#include "NotifyGVContactsTable.h"
#include "ContactsParserObject.h"

GVContactsTable::GVContactsTable (QObject *parent)
: QObject (parent)
, api(this)
, mutex(QMutex::Recursive)
, bRefreshRequested (false)
{
    connect(&api, SIGNAL(oneContact(ContactInfo)),
            this, SLOT(gotOneContact(ContactInfo)));
}//GVContactsTable::GVContactsTable

GVContactsTable::~GVContactsTable ()
{
}//GVContactsTable::~GVContactsTable

void
GVContactsTable::login(const QString &strU, const QString &strP)
{
    QMutexLocker locker(&mutex);
    strUser = strU;
    strPass = strP;

    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate contacts task");
        qApp->quit ();
        return;
    }

    task->inParams["user"] = strUser;
    task->inParams["pass"] = strPass;

    if (!api.login (task)) {
        Q_WARN("Failed to start contacts login");
        qApp->quit ();
        return;
    }
}//GVContactsTable::loginSuccess

void
GVContactsTable::logout ()
{
    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to logout");
        return;
    }

    connect(task, SIGNAL(completed()), task, SLOT(deleteLater()));
    if (!api.logout (task)) {
        Q_WARN("Failed to logout");
        delete task;
    }
}//GVContactsTable::logout

void
GVContactsTable::onPresentCaptcha(AsyncTaskToken *task,
                                  const QString & /*captchaUrl*/)
{
    Q_WARN("No way to handle a captcha!!");
    task->deleteLater ();
    qApp->quit ();
}//GVContactsTable::onPresentCaptcha

void
GVContactsTable::onLoginDone()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();

    if (NULL == task) {
        Q_WARN("invalid task");
        qApp->quit();
        return;
    }

    if (ATTS_SUCCESS != task->status) {
        Q_WARN("Failed to login to contacts");
        task->deleteLater ();
        qApp->quit();
        return;
    }

    QMutexLocker locker (&mutex);
    if (bRefreshRequested) {
        refreshContacts ();
    }

    task->deleteLater ();
}//GVContactsTable::onLoginDone

void
GVContactsTable::refreshContacts ()
{
    QMutexLocker locker(&mutex);
    if (!api.isLoggedIn ()) {
        bRefreshRequested = true;
        return;
    }

    AsyncTaskToken *task = new AsyncTaskToken(this);
    if (NULL == task) {
        Q_WARN("Failed to allocate task for refresh");
        qApp->quit();
        return;
    }

    bool showDeleted = true;
    task->inParams["updatedMin"] = dtUpdate;
    task->inParams["showDeleted"] = showDeleted;
    connect(task, SIGNAL(completed()), this, SLOT(onContactsParsed()));
    if (!api.getContacts (task)) {
        Q_WARN("Failed to get contacts");
        delete task;
        qApp->quit ();
        return;
    } else {
        bRefreshRequested = false;
        bChangedSinceRefresh = false;
    }
}//GVContactsTable::refreshContacts

void
GVContactsTable::gotOneContact (ContactInfo contactInfo)
{
    QMutexLocker locker(&mutex);
    if (contactInfo.dtUpdate > dtUpdate) {
        dtUpdate = contactInfo.dtUpdate;
        bChangedSinceRefresh = true;
    }
}//GVContactsTable::gotOneContact

void
GVContactsTable::onContactsParsed ()
{
    AsyncTaskToken *task = (AsyncTaskToken *) QObject::sender ();

    if (NULL == task) {
        Q_WARN("invalid task");
        qApp->quit();
        return;
    }

    emit allContacts (bChangedSinceRefresh, (ATTS_SUCCESS == task->status));
}//GVContactsTable::onContactsParsed
