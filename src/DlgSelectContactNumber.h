#ifndef __DLGSELECTCONTACTNUMBER_H__
#define __DLGSELECTCONTACTNUMBER_H__

#include "global.h"

// For some reason the symbian MOC doesn't like it if I don't include QObject
// even though it is present in QtCore which is included in global.h
#include <QObject>

class DlgSelectContactNumber : public QDialog
{
public:
    DlgSelectContactNumber(const GVContactInfo     &info      ,
                                 QWidget           *parent = 0,
                                 Qt::WindowFlags    f      = 0);
    ~DlgSelectContactNumber(void);

    int getSelection ();

private:
    QVector<QRadioButton *> arrRadios;
    QPushButton btnOk;
    QPushButton btnCancel;
    QGridLayout grid;
};

#endif //__DLGSELECTCONTACTNUMBER_H__
