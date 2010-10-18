#ifndef INBOXTREEVIEW_H
#define INBOXTREEVIEW_H

#include "global.h"

class InboxTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit InboxTreeView(QWidget *parent = 0);

private:
    void selectionChanged (const QItemSelection &selected,
                           const QItemSelection &deselected);

public:
    //! Contact ID of the currently selected inbox entry
    QString         strContactId;
    //! More details about the currently selected inbox entry
    GVHistoryEvent  historyEvent;
};

#endif // INBOXTREEVIEW_H
