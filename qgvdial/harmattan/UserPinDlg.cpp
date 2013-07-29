#include "UserPinDlg.h"
#include "MTextEdit"

UserPinDlg::UserPinDlg()
: MDialog()
{
    setModal (true);
    setSystem (false);
    setTitle ("Two factor authentication");

    MTextEdit *pin = new MTextEdit(MTextEditModel::SingleLine);
    pin->setPrompt ("Two factor PIN");
    setCentralWidget (pin);
}//UserPinDlg::UserPinDlg
