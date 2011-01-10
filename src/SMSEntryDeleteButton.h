#ifndef __SMSENTRYDELETEBUTTON_H__
#define __SMSENTRYDELETEBUTTON_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class SMSEntryDeleteButton : public QPushButton
{
    Q_OBJECT

public:
    SMSEntryDeleteButton (int ind, QWidget *parent = 0);
    ~SMSEntryDeleteButton (void);

    void setIndex (int i);

signals:
    void triggered (int index);

private slots:
    void btnClicked ();

private:
    int index;
};

#endif //__SMSENTRYDELETEBUTTON_H__
