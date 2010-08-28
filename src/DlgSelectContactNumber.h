#ifndef __DLGSELECTCONTACTNUMBER_H__
#define __DLGSELECTCONTACTNUMBER_H__

#include "global.h"

#include <QtCore>
#include <QtGui>

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
