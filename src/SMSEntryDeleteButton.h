#ifndef __SMSENTRYDELETEBUTTON_H__
#define __SMSENTRYDELETEBUTTON_H__

#include "global.h"

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
