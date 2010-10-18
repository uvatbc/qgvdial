#ifndef CONTACTSTREEVIEW_H
#define CONTACTSTREEVIEW_H

#include <QtGui>

class ContactsTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ContactsTreeView(QWidget *parent = 0);

private:
    void selectionChanged (const QItemSelection &selected,
                           const QItemSelection &deselected);

public:
    //! The link to the current contact
    QString         strSavedLink;
};

#endif // CONTACTSTREEVIEW_H
