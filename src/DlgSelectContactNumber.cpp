#include "DlgSelectContactNumber.h"

DlgSelectContactNumber::DlgSelectContactNumber(
    const GVContactInfo  &info          ,
          QWidget        *parent/* = 0*/,
          Qt::WindowFlags f     /* = 0*/):
QDialog(parent, f),
btnOk("OK", this),
btnCancel("Cancel", this)
{
    this->setWindowTitle (info.strName);
    int i;
    for (i = 0; i < info.arrPhones.count (); i++)
    {
        QLabel       *pType  = new QLabel (QString (info.arrPhones[i].chType),
                                           this);
        QRadioButton *pRadio = new QRadioButton (info.arrPhones[i].strNumber,
                                                 this);
        arrRadios += pRadio;
        grid.addWidget (pType , i, 0);
        grid.addWidget (pRadio, i, 1);
        if (info.selected == i)
        {
            pRadio->setChecked (true);
        }
    }

    QObject::connect (&btnOk    , SIGNAL (clicked ()),
                       this     , SLOT   (accept ()));
    QObject::connect (&btnCancel, SIGNAL (clicked ()),
                       this     , SLOT   (reject ()));

    grid.addWidget (&btnOk, i, 0);
    grid.addWidget (&btnCancel, i, 1);

    this->setLayout (&grid);
}//DlgSelectContactNumber::DlgSelectContactNumber

DlgSelectContactNumber::~DlgSelectContactNumber(void)
{
}//DlgSelectContactNumber::~DlgSelectContactNumber

int
DlgSelectContactNumber::getSelection ()
{
    for (int i = 0; i < arrRadios.count () ; i++)
    {
        if (arrRadios[i]->isChecked ())
        {
            return (i);
        }
    }

    return (-1);
}//DlgSelectContactNumber::getSelection
