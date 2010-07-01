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
DialCancelDlg::doModal ()
{
    ObserverFactory &obsFactory = ObserverFactory::getRef ();
    listObservers = obsFactory.createObservers (strContact);
    foreach (IObserver *observer, listObservers)
    {
        QObject::connect (observer, SIGNAL (callStarted()),
                          this    , SLOT   (accept()));
    }

    return (this->exec ());
}//DialCancelDlg::doModal
