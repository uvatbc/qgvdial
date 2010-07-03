#include "DialCancelDlg.h"

DialCancelDlg::DialCancelDlg (const QString &strNum, QWidget *parent)
: QMessageBox (parent)
, strContact (strNum)
{
    this->setText (QString("Dialing %1. Abort / Cancel ends the call")
                   .arg(strContact));
    this->setStandardButtons (QMessageBox::Ok | QMessageBox::Abort);
    this->setDefaultButton (QMessageBox::Ok);
}//DialCancelDlg::DialCancelDlg

int
DialCancelDlg::doModal (const QString &strMyNumber)
{
    ObserverFactory &obsFactory = ObserverFactory::getRef ();
    obsFactory.startObservers (strMyNumber, this, SLOT (callStarted()));

    int rv = this->exec ();

    obsFactory.stopObservers ();

    return (rv);
}//DialCancelDlg::doModal

void
DialCancelDlg::callStarted ()
{
    this->done (QMessageBox::Ok);
}//DialCancelDlg::callStarted
