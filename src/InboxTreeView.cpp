#include "InboxTreeView.h"
#include "Singletons.h"

InboxTreeView::InboxTreeView (QWidget *parent)
: QTreeView(parent)
{
}//InboxTreeView::InboxTreeView

void
InboxTreeView::selectionChanged (const QItemSelection &selected,
                                 const QItemSelection &deselected)
{
    do // Begin cleanup block (not a loop)
    {
        QModelIndexList indList = selected.indexes ();
        if (0 == indList.size ())
        {
            qWarning ("Empty selection");
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

}//InboxTreeView::selectionChanged
