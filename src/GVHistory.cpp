#include "global.h"
#include "GVHistory.h"
#include "Singletons.h"
#include "InboxModel.h"

#include "ui_InboxWidget.h"

GVHistory::GVHistory (QWidget *parent, Qt::WindowFlags flags)
: QMainWindow (parent, flags)
, ui (new Ui::InboxWidget)
, actionGroup (this)
, mnuContext ("Action", this)
, mutex (QMutex::Recursive)
, bLoggedIn (false)
{
    ui->setupUi (this);

    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setDefaultWindowAttributes (this);

    // Connect the log to this classes log.
    QObject::connect (
        ui->treeView, SIGNAL (log(const QString &, int)),
        this        , SIGNAL (log(const QString &, int)));

    // Add all these actions to the action group
    actionGroup.addAction (ui->actionAll);
    actionGroup.addAction (ui->actionPlaced);
    actionGroup.addAction (ui->actionReceived);
    actionGroup.addAction (ui->actionMissed);
    actionGroup.addAction (ui->actionVoicemail);
    actionGroup.addAction (ui->actionSMS);
    actionGroup.setExclusive (true);

    // Initially, all are to be selected
    ui->actionAll->setChecked (true);
    strSelectedMessages = "all";

    // actionGroup.trigger -> this.onInboxSelected
    QObject::connect (
        &actionGroup, SIGNAL (triggered       (QAction *)),
         this       , SLOT   (onInboxSelected (QAction *)));

    // The status must be shown on this window as well
    QObject::connect (
        this, SIGNAL (status    (const QString &, int)),
        this, SLOT   (setStatus (const QString &, int)));
}//GVHistory::GVHistory

GVHistory::~GVHistory(void)
{
    deinitModel ();
}//GVHistory::~GVHistory

void
GVHistory::deinitModel ()
{
    ui->treeView->reset ();

    InboxModel *modelInbox = (InboxModel *) ui->treeView->model ();
    ui->treeView->setModel (NULL);
    if (NULL != modelInbox)
    {
        delete modelInbox;
        modelInbox = NULL;
    }
}//GVHistory::deinitModel

void
GVHistory::initModel ()
{
    deinitModel ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    InboxModel *modelInbox = dbMain.newInboxModel ();
    ui->treeView->setModel (modelInbox);
    prepView ();
}//GVHistory::initModel

void
GVHistory::prepView ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    InboxModel *inboxModel = (InboxModel *) ui->treeView->model ();

    emit status ("Re-selecting inbox entries. This will take some time", 0);
    dbMain.refreshInboxModel (inboxModel, strSelectedMessages);
    emit status ("Inbox entries selected.");

    ui->treeView->hideColumn (0);
    ui->treeView->hideColumn (5);
    ui->treeView->sortByColumn (2);

    for (int i = 0; i < 5; i++)
    {
        ui->treeView->resizeColumnToContents (i);
    }
}//GVHistory::prepView

void
GVHistory::setStatus (const QString &strText, int timeout)
{
    ui->statusbar->showMessage (strText, timeout);
}//GVHistory::setStatus

void
GVHistory::refreshHistory ()
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QDateTime dtUpdate;
    dbMain.getLastInboxUpdate (dtUpdate);

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QVariantList l;
    l += "all";
    l += "1";
    l += "10";
    l += dtUpdate;
    QObject::connect (
        &webPage, SIGNAL (oneHistoryEvent (const GVHistoryEvent &)),
         this   , SLOT   (oneHistoryEvent (const GVHistoryEvent &)));
    emit status ("Retrieving Inbox...", 0);
    if (!webPage.enqueueWork (GVAW_getInbox, l, this,
            SLOT (getHistoryDone (bool, const QVariantList &))))
    {
        getHistoryDone (false, l);
    }
}//GVHistory::refreshHistory

void
GVHistory::refreshFullInbox ()
{
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setLastInboxUpdate (QDateTime::fromString ("2000-01-01",
                                                      "yyyy-MM-dd"));
    dbMain.clearInbox ();
    refreshHistory ();
}//GVHistory::refreshFullInbox

void
GVHistory::oneHistoryEvent (const GVHistoryEvent &hevent)
{
    InboxModel *tModel = (InboxModel *) ui->treeView->model ();
    QString     strType = tModel->type_to_string (hevent.Type);
    if (0 == strType.size ())
    {
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.insertHistory (hevent);
}//GVHistory::oneHistoryEvent

void
GVHistory::getHistoryDone (bool, const QVariantList &)
{
    emit status ("Inbox retrieved. Sorting...", 0);

    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (oneHistoryEvent (const GVHistoryEvent &)),
         this   , SLOT   (oneHistoryEvent (const GVHistoryEvent &)));

    prepView ();

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    QDateTime dtUpdate;
    if (dbMain.getLatestInboxEntry (dtUpdate))
    {
        emit log (QString ("Latest inbox entry is : %1")
                  .arg (dtUpdate.toString ()));
        dbMain.setLastInboxUpdate (dtUpdate);
    }

    emit status ("Inbox ready");
}//GVHistory::getHistoryDone

void
GVHistory::onInboxSelected (QAction *action)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    strSelectedMessages = action->text().toLower ();
    refreshHistory ();
}//GVHistory::onInboxSelected

void
GVHistory::loginSuccess ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = true;
}//GVHistory::loginSuccess

void
GVHistory::loggedOut ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = false;

    ui->treeView->setModel (NULL);
}//GVHistory::loggedOut

void
GVHistory::contextMenuEvent (QContextMenuEvent * event)
{
    mnuContext.clear ();
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == ui->treeView->historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }

    if (GVHE_Voicemail == ui->treeView->historyEvent.Type)
    {
        mnuContext.addAction (ui->actionPlay_voicemail);
    }
    mnuContext.addAction (ui->actionCall);
    mnuContext.addAction (ui->actionSend_Text);

    mnuContext.popup (event->globalPos ());
}//GVHistory::contextMenuEvent

void
GVHistory::placeCall ()
{
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == ui->treeView->historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }

    if (0 == ui->treeView->strContactId.size ())
    {
        // In case it's an unknown number, call by number
        emit callNumber (ui->treeView->historyEvent.strPhoneNumber);
    }
    else
    {
        emit callNumber (ui->treeView->historyEvent.strPhoneNumber,
                         ui->treeView->strContactId);
    }
}//GVHistory::placeCall

void
GVHistory::sendSMS ()
{
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == ui->treeView->historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }

    if (0 == ui->treeView->strContactId.size ())
    {
        // In case it's an unknown number, SMS by number
        emit textANumber (ui->treeView->historyEvent.strPhoneNumber);
    }
    else
    {
        emit textANumber (ui->treeView->historyEvent.strPhoneNumber,
                          ui->treeView->strContactId);
    }
}//GVHistory::sendSMS

void
GVHistory::playVoicemail ()
{
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == ui->treeView->historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }
    if (GVHE_Voicemail != ui->treeView->historyEvent.Type)
    {
        emit status ("Not a voice mail.");
        return;
    }

    emit retrieveVoicemail (ui->treeView->historyEvent.id);
}//GVHistory::playVoicemail
