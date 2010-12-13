#include "ContactsTreeView.h"

ContactsTreeView::ContactsTreeView (QWidget *parent)
: QTreeView(parent)
{
}//ContactsTreeView::ContactsTreeView

void
ContactsTreeView::selectionChanged (const QItemSelection &selected,
                                    const QItemSelection & /*deselected*/)
{
    QModelIndexList lstModels = selected.indexes ();
    if (0 == lstModels.size ())
    {
        strSavedLink.clear ();
        return;
    }
    QModelIndex linkIndex = lstModels[0].model()->index(lstModels[0].row(),1);
    strSavedLink = linkIndex.data(Qt::EditRole).toString();
    if (strSavedLink.isEmpty ())
    {
        qWarning ("Failed to get contact information");
    }
}//ContactsTreeView::selectionChanged
