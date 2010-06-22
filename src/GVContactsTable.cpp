#include "global.h"
#include "GVContactsTable.h"
#include "GVWebPage.h"

GVContactsTable::GVContactsTable (CacheDatabase &db, QWidget *parent) :
QTreeView(parent),
dbMain(db),
actRefresh("&Refresh", this),
mnuContext("Action", this),
actPlaceCall("Call", this),
actSendSMS("SMS", this),
mutex(QMutex::Recursive),
bLoggedIn(false)
{
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

    // Prepare the menu actions
    mnuContext.addAction (&actPlaceCall);
    mnuContext.addAction (&actSendSMS);

    // actPlaceCall.triggered -> this.placeCall
    QObject::connect (&actPlaceCall, SIGNAL (triggered ()),
                       this        , SLOT   (placeCall ()));
    // actSendSMS.triggered -> this.sendSMS
    QObject::connect (&actSendSMS, SIGNAL (triggered ()),
                       this      , SLOT   (sendSMS ()));

    QKeySequence keyRefresh(Qt::ControlModifier + Qt::Key_R);
    actRefresh.setShortcut (keyRefresh);
    QObject::connect (&actRefresh, SIGNAL (triggered ()),
                       this      , SLOT   (refreshContacts ()));

    // this.activated -> this.activatedContact
    QObject::connect (
        this, SIGNAL (activated        (const QModelIndex &)),
        this, SLOT   (activatedContact (const QModelIndex &)));
}//GVContactsTable::GVContactsTable

void
GVContactsTable::refreshContacts ()
{
    QMutexLocker locker(&mutex);
    dbMain.clearContacts ();
    strSavedLink.clear ();

    GVWebPage &webPage = GVWebPage::getRef ();
    QVariantList l;
    nContacts = 0;
    QObject::connect (
        &webPage, SIGNAL (gotContact (const QString &, const QString &)),
         this   , SLOT   (gotContact (const QString &, const QString &)));
    emit status ("Retrieving all contacts...", 0);
    if (!webPage.enqueueWork (GVWW_getAllContacts, l, this,
            SLOT (getContactsDone (bool, const QVariantList &))))
    {
        getContactsDone (false, l);
    }
}//GVContactsTable::refreshContacts

void
GVContactsTable::gotContact (const QString &strName, const QString &strLink)
{
    emit oneContact (nContacts, strName, strLink);

    QMutexLocker locker(&mutex);
    nContacts++;
}//GVContactsTable::gotContact

void
GVContactsTable::getContactsDone (bool bOk, const QVariantList &)
{
    GVWebPage &webPage = GVWebPage::getRef ();
    QObject::disconnect(
        &webPage, SIGNAL (gotContact(const QString &, const QString &)),
        this    , SLOT   (gotContact(const QString &, const QString &)));

    emit allContacts (bOk);
}//GVContactsTable::getContactsDone

void
GVContactsTable::updateMenu (QMenuBar *menuBar)
{
    QMutexLocker locker(&mutex);
    if (!bLoggedIn)
    {
        return;
    }

    menuBar->addAction (&actRefresh);
    menuBar->addAction (&actPlaceCall);
    menuBar->addAction (&actSendSMS);
}//GVContactsTable::updateMenu

void
GVContactsTable::loginSuccess ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = true;
}//GVContactsTable::loginSuccess

void
GVContactsTable::loggedOut ()
{
    QMutexLocker locker(&mutex);
    bLoggedIn = false;
}//GVContactsTable::loggedOut

void
GVContactsTable::activatedContact (const QModelIndex &)
{
    placeCall ();
}//GVContactsTable::activatedContact

void
GVContactsTable::selectionChanged (const QItemSelection &selected,
                                   const QItemSelection &/*deselected*/)
{
    QModelIndexList listModels = selected.indexes ();
    if (0 == listModels.size ())
    {
        strSavedLink.clear ();
        return;
    }
    QModelIndex linkIndex = listModels[0].model()->index (listModels[0].row(),
                                                          1);
    strSavedLink = linkIndex.data(Qt::EditRole).toString();
    if (strSavedLink.isEmpty ())
    {
        log ("Failed to get contact information", 3);
    }
}//GVContactsTable::selectionChanged

void
GVContactsTable::contextMenuEvent (QContextMenuEvent * event)
{
    mnuContext.popup (event->globalPos ());
}//GVContactsTable::contextMenuEvent

void
GVContactsTable::placeCall ()
{
    QMutexLocker locker(&mutex);
    if (0 != strSavedLink.size ())
    {
        emit callNameLink (strSavedLink, "");
    }
    else
    {
        emit status ("Nothing selected");
    }
}//GVContactsTable::placeCall

void
GVContactsTable::sendSMS ()
{
    QMutexLocker locker(&mutex);
    if (0 != strSavedLink.size ())
    {
        emit sendSMSToNameLink (strSavedLink, "");
    }
    else
    {
        emit status ("Nothing selected");
    }
}//GVContactsTable::sendSMS
