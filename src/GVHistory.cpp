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
    strSelected = "all";

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
    // actPlayVmail.triggered -> this.playVoicemail
    QObject::connect (&actPlayVmail, SIGNAL (triggered     ()),
                       this        , SLOT   (playVoicemail ()));

    // Not modifyable
    this->setEditTriggers (QAbstractItemView::NoEditTriggers);
    // Only one item selectable at a time.
    this->setSelectionMode (QAbstractItemView::SingleSelection);
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
    //clearAllItems ();

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

QString
GVHistory::type_to_string (GVH_Event_Type Type)
{
    QString strReturn;
    switch (Type)
    {
    case GVHE_Placed:
        strReturn = "Placed";
        break;
    case GVHE_Received:
        strReturn = "Received";
        break;
    case GVHE_Missed:
        strReturn = "Missed";
        break;
    case GVHE_Voicemail:
        strReturn = "Voicemail";
        break;
    case GVHE_TextMessage:
        strReturn = "SMS";
        break;
    default:
        break;
    }
    return (strReturn);
}//GVHistory::type_to_string

GVH_Event_Type
GVHistory::string_to_type (const QString &strType)
{
    GVH_Event_Type Type = GVHE_Unknown;

    do // Begin cleanup block (not a loop)
    {
        if (0 == strType.compare ("Placed"))
        {
            Type = GVHE_Placed;
            break;
        }
        if (0 == strType.compare ("Received"))
        {
            Type = GVHE_Received;
            break;
        }
        if (0 == strType.compare ("Missed"))
        {
            Type = GVHE_Missed;
            break;
        }
        if (0 == strType.compare ("Voicemail"))
        {
            Type = GVHE_Voicemail;
            break;
        }
        if (0 == strType.compare ("SMS"))
        {
            Type = GVHE_TextMessage;
            break;
        }
    } while (0); // End cleanup block (not a loop)

    return (Type);
}//GVHistory::string_to_type

void
GVHistory::oneHistoryEvent (const GVHistoryEvent &hevent)
{
    QStringList arrItems;
    QString     strType = type_to_string (hevent.Type);
    if (0 == strType.size ())
    {
        return;
    }

    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    InboxModel *tModel = (InboxModel *) this->model ();
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

    this->hideColumn (0);
    this->hideColumn (4);
    this->hideColumn (5);

    for (int i = 0; i < 4; i++)
    {
        this->resizeColumnToContents (i);
    }

    QDateTime dtUpdate = QDateTime::currentDateTime().toUTC ();
    CacheDatabase &dbMain = Singletons::getRef().getDBMain ();
    dbMain.setLastInboxUpdate (dtUpdate);
}//GVHistory::getHistoryDone

void
GVHistory::onCurrentItemChanged (QTreeWidgetItem *current,
                                 QTreeWidgetItem * /*previous*/)
{
    QMutexLocker locker(&mutex);
    if (NULL == current)
    {
        GVHistoryEvent blank;
        historyEvent = blank;
        return;
    }

    // To ensure that we get to the topmost item - required for SMS items
    while (NULL != current->parent ())
    {
        current = current->parent ();
    }

    QVariant var = current->data(5, Qt::EditRole);
    if (var.canConvert <GVHistoryEvent> ())
    {
        historyEvent = var.value <GVHistoryEvent> ();
    }
}//GVHistory::onCurrentItemChanged

void
GVHistory::onItemActivated (QTreeWidgetItem * /*item*/, int /* column*/)
{
    placeCall ();
}//GVHistory::onItemActivated

void
GVHistory::onInboxSelected (QAction *action)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    strSelected = action->text().toLower ();
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

    if (0 == historyEvent.id.size ())
    {
        // In case it's an unknown number, call by link
        emit callLink (historyEvent.id);
    }
    else
    {
        emit callNameLink (historyEvent.id, historyEvent.strPhoneNumber);
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

    if (0 == historyEvent.id.size ())
    {
        // In case it's an unknown number, SMS by link
        emit sendSMSToLink (historyEvent.id);
    }
    else
    {
        emit sendSMSToNameLink (historyEvent.id,
                                historyEvent.strPhoneNumber);
    }
}//GVHistory::sendSMS

void
GVHistory::playVoicemail ()
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
}//GVHistory::playVoicemail
