#include "ContactsTreeView.h"

ContactsTreeView::ContactsTreeView (QWidget *parent)
: QTreeView(parent)
{
}//ContactsTreeView::ContactsTreeView

void
ContactsTreeView::selectionChanged (const QItemSelection &selected,
                                    const QItemSelection & /*deselected*/)
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
        qWarning ("Failed to get contact information");
    }
}//ContactsTreeView::selectionChanged
