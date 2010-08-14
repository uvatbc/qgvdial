#include "global.h"
#include "GVHistory.h"
#include "Singletons.h"
#include "InboxModel.h"

GVHistory::GVHistory (QWidget *parent/* = 0*/)
: QTreeView (parent)
, actRefresh ("&Refresh", this)
, mnuSelectInbox ("Select", this)
, menubarActions ("Event actions", this)
, actionGroup (this)
, actSelectAll      ("All", this)
, actSelectPlaced   ("Placed", this)
, actSelectReceived ("Received", this)
, actSelectMissed   ("Missed", this)
, actSelectVoicemail("Voicemail", this)
, actSelectSMS      ("SMS", this)
, mnuContext("Action", this)
, actPlaceCall  ("Call", this)
, actSendSMS    ("SMS", this)
, actPlayVmail  ("Play", this)
, mutex (QMutex::Recursive)
, bLoggedIn (false)
{
    // Menu item for this page
    QKeySequence keyRefresh(Qt::ControlModifier + Qt::Key_R);
    actRefresh.setShortcut (keyRefresh);
    QObject::connect (&actRefresh, SIGNAL (triggered ()),
                       this      , SLOT   (refreshHistory ()));

    actSelectAll.setCheckable (true);
    actSelectPlaced.setCheckable (true);
    actSelectReceived.setCheckable (true);
    actSelectMissed.setCheckable (true);
    actSelectVoicemail.setCheckable (true);
    actSelectSMS.setCheckable (true);

    // Add all these actions to the action group
    actionGroup.addAction (&actSelectAll);
    actionGroup.addAction (&actSelectPlaced);
    actionGroup.addAction (&actSelectReceived);
    actionGroup.addAction (&actSelectMissed);
    actionGroup.addAction (&actSelectVoicemail);
    actionGroup.addAction (&actSelectSMS);
    actionGroup.setExclusive (true);

    // Select history menu
    mnuSelectInbox.addAction (&actSelectAll);
    mnuSelectInbox.addAction (&actSelectPlaced);
    mnuSelectInbox.addAction (&actSelectReceived);
    mnuSelectInbox.addAction (&actSelectMissed);
    mnuSelectInbox.addAction (&actSelectVoicemail);
    mnuSelectInbox.addAction (&actSelectSMS);

    // Menu bar menu action
    menubarActions.addAction (&actPlaceCall);
    menubarActions.addAction (&actSendSMS);
    menubarActions.addAction (&actPlayVmail);

    // Initially, all are to be selected
    actSelectAll.setChecked (true);
    strSelectedMessages = "all";

    // Double click OR Enter press on an item
    QObject::connect (
        this, SIGNAL (itemActivated   (QTreeWidgetItem *, int)),
        this, SLOT   (onItemActivated (QTreeWidgetItem *, int)));

    // User navigates to an item
    QObject::connect (
        this,
        SIGNAL (currentItemChanged (QTreeWidgetItem *, QTreeWidgetItem *)),
        this,
        SLOT   (onCurrentItemChanged (QTreeWidgetItem *, QTreeWidgetItem *)));

    // actionGroup.trigger -> this.onInboxSelected
    QObject::connect (
        &actionGroup, SIGNAL (triggered       (QAction *)),
         this       , SLOT   (onInboxSelected (QAction *)));

    // actPlaceCall.triggered -> this.placeCall
    QObject::connect (&actPlaceCall, SIGNAL (triggered ()),
                       this        , SLOT   (placeCall ()));
    // actSendSMS.triggered -> this.sendSMS
    QObject::connect (&actSendSMS, SIGNAL (triggered ()),
                       this      , SLOT   (sendSMS ()));
    // actPlayVmail.triggered -> this.onactPlayVmailTriggered
    QObject::connect (&actPlayVmail, SIGNAL (triggered             ()),
                       this        , SLOT   (actPlayVmailTriggered ()));

    // Not modifyable
    this->setEditTriggers (QAbstractItemView::NoEditTriggers);
    // Only one item selectable at a time.
    this->setSelectionMode (QAbstractItemView::SingleSelection);
    // Select rows at a time
    this->setSelectionBehavior (QAbstractItemView::SelectRows);
    // Alternating colors = on
    this->setAlternatingRowColors (true);
    // Don't show the (un)collapse sign
    this->setRootIsDecorated (false);
    // Hide the header
    this->setHeaderHidden (true);
}//GVHistory::GVHistory

GVHistory::~GVHistory(void)
{
    this->setModel (NULL);
}//GVHistory::~GVHistory

void
GVHistory::updateMenu (QMenuBar *menuBar)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    menuBar->addAction (&actRefresh);
    menuBar->addMenu (&mnuSelectInbox);
    menuBar->addMenu (&menubarActions);
}//GVHistory::updateMenu

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
    emit status ("Retrieving history events...");
    if (!webPage.enqueueWork (GVAW_getInbox, l, this,
            SLOT (getHistoryDone (bool, const QVariantList &))))
    {
        getHistoryDone (false, l);
    }
}//GVHistory::refreshHistory

void
GVHistory::oneHistoryEvent (const GVHistoryEvent &hevent)
{
    InboxModel *tModel = (InboxModel *) this->model ();
    QString     strType = tModel->type_to_string (hevent.Type);
    if (0 == strType.size ())
    {
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.insertHistory (tModel, hevent);
}//GVHistory::oneHistoryEvent

void
GVHistory::getHistoryDone (bool, const QVariantList &)
{
    GVAccess &webPage = Singletons::getRef().getGVAccess ();
    QObject::disconnect (
        &webPage, SIGNAL (oneHistoryEvent (const GVHistoryEvent &)),
         this   , SLOT   (oneHistoryEvent (const GVHistoryEvent &)));

    InboxModel *tModel = (InboxModel *) this->model ();
    tModel->submitAll ();
    tModel->selectOnly (strSelectedMessages);

    this->hideColumn (0);
    this->hideColumn (5);
    this->sortByColumn (2);

    for (int i = 0; i < 4; i++)
    {
        this->resizeColumnToContents (i);
    }

    QDateTime dtUpdate = QDateTime::currentDateTime().toUTC ();
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setLastInboxUpdate (dtUpdate);
}//GVHistory::getHistoryDone

void
GVHistory::selectionChanged (const QItemSelection &selected,
                             const QItemSelection &deselected)
{
    do // Begin cleanup block (not a loop)
    {
        QModelIndexList indList = selected.indexes ();
        if (0 == indList.size ())
        {
            emit log ("Empty selection", 5);
            break;
        }

        QModelIndex index = indList[0];
        while (index.parent ().isValid ())
        {
            index = index.parent ();
        }

        QAbstractItemModel *model = this->model();

        // Init to blank
        historyEvent.init ();
        strContactId.clear ();

        // Get the extry id
        historyEvent.id =
            model->index(index.row(),0).data(Qt::EditRole).toString ();
        historyEvent.Type = (GVH_Event_Type)
            model->index(index.row(),1).data(Qt::EditRole).toChar().toAscii ();
        historyEvent.startTime =
            model->index(index.row(),2).data(Qt::EditRole).toDateTime ();
        historyEvent.strDisplayNumber =
            model->index(index.row(),3).data(Qt::EditRole).toString ();
        historyEvent.strPhoneNumber =
            model->index(index.row(),4).data(Qt::EditRole).toString ();
        quint32 flags =
            model->index(index.row(),5).data(Qt::EditRole).toInt ();

        historyEvent.bRead  = (flags & (1 << 0)) ? true : false;
        historyEvent.bSpam  = (flags & (1 << 1)) ? true : false;
        historyEvent.bTrash = (flags & (1 << 2)) ? true : false;
        historyEvent.bStar  = (flags & (1 << 3)) ? true : false;

        // Get the Phone number
        QString strNumber = historyEvent.strPhoneNumber;

        // If it isn't a valid number, we won't be able to pull information
        if (!GVAccess::isNumberValid (strNumber)) break;

        // Get info about this contact
        GVAccess::simplify_number (strNumber);
        CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
        GVContactInfo info;
        if (!dbMain.getContactFromNumber (strNumber, info)) break;

        // Info found!
        strContactId = info.strLink;
    } while (0); // End cleanup block (not a loop)

    QTreeView::selectionChanged (selected, deselected);
}//GVHistory::selectionChanged

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

    this->setModel (NULL);
}//GVHistory::loggedOut

void
GVHistory::contextMenuEvent (QContextMenuEvent * event)
{
    mnuContext.clear ();
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }

    if (GVHE_Voicemail == historyEvent.Type)
    {
        mnuContext.addAction (&actPlayVmail);
    }
    mnuContext.addAction (&actPlaceCall);
    mnuContext.addAction (&actSendSMS);

    mnuContext.popup (event->globalPos ());
}//GVHistory::contextMenuEvent

void
GVHistory::placeCall ()
{
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }

    if (0 == strContactId.size ())
    {
        // In case it's an unknown number, call by number
        emit callNumber (historyEvent.strPhoneNumber);
    }
    else
    {
        emit callNumber (historyEvent.strPhoneNumber, strContactId);
    }
}//GVHistory::placeCall

void
GVHistory::sendSMS ()
{
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }

    if (0 == strContactId.size ())
    {
        // In case it's an unknown number, SMS by number
        emit textANumber (historyEvent.strPhoneNumber);
    }
    else
    {
        emit textANumber (historyEvent.strPhoneNumber, strContactId);
    }
}//GVHistory::sendSMS

void
GVHistory::actPlayVmailTriggered ()
{
    QMutexLocker locker(&mutex);
    if (GVHE_Unknown == historyEvent.Type)
    {
        emit status ("Nothing selected.");
        return;
    }
    if (GVHE_Voicemail != historyEvent.Type)
    {
        emit status ("Not a voice mail.");
        return;
    }

    emit playVoicemail (historyEvent.id);
}//GVHistory::onactPlayVmailTriggered
