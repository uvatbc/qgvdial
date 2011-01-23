#include "DialCancelDlg.h"
#include "Singletons.h"

DialCancelDlg::DialCancelDlg(const QString &strNum, void *ctx, QWidget *parent)
: QMessageBox (parent)
, strContact (strNum)
, m_Ctx (ctx)
{
    OsDependent &osd = Singletons::getRef().getOSD ();
    osd.setDefaultWindowAttributes (this);

    this->setText (QString("Dialing %1. Abort / Cancel ends the call")
                   .arg(strContact));
    this->setStandardButtons (QMessageBox::Ok | QMessageBox::Abort);
    this->setDefaultButton (QMessageBox::Ok);
}//DialCancelDlg::DialCancelDlg

int
DialCancelDlg::doModal (const QString &strMyNumber)
{
    ObserverFactory &obsF = Singletons::getRef().getObserverFactory ();
    obsF.startObservers (strMyNumber, this, SLOT (callStarted()));

    int rv = this->exec ();

    return (rv);
}//DialCancelDlg::doModal

void
DialCancelDlg::callStarted ()
{
    this->done (QMessageBox::Ok);
}//DialCancelDlg::callStarted

void
DialCancelDlg::doNonModal (const QString &strMyNumber)
{
    ObserverFactory &obsF = Singletons::getRef().getObserverFactory ();
    obsF.startObservers (strMyNumber, this, SLOT (callStarted()));

    this->show ();
}//DialCancelDlg::doNonModal

void
DialCancelDlg::done (int r)
{
    ObserverFactory &obsF = Singletons::getRef().getObserverFactory ();
    obsF.stopObservers ();

    emit dialDlgDone (r, strContact, m_Ctx);

    QMessageBox::done (r);
}//DialCancelDlg::done
