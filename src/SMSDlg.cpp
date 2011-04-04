#include "SMSDlg.h"

SMSDlg::SMSDlg(QWidget * parent/* = 0*/, Qt::WindowFlags f/* = 0*/)
: ChildWindowBase (parent, f)
, edSMSText (this)
, lblTextCount ("Letters = 0", this)
, btnSendSMS ("Send", this)
{
    // btnSendSMS.clicked -> this.btnSendSMSClicked
    QObject::connect (&btnSendSMS, SIGNAL (clicked ()),
                       this      , SLOT   (btnSendSMSClicked ()));
    // edSMSText.textChanged -> this.smsTextChanged
    QObject::connect (&edSMSText, SIGNAL (textChanged ()),
                       this     , SLOT   (smsTextChanged ()));
}//SMSDlg::SMSDlg

SMSDlg::~SMSDlg(void)
{
}//SMSDlg::~SMSDlg

bool
SMSDlg::addSMSEntry (const SMSEntry &entry)
{
    QString strNumberinfo = QString ("%1 - %2").arg (entry.sNumber.strNumber)
                      .arg (PhoneInfo::typeToString (entry.sNumber.Type));

    int index = smsGuiEntries.size();
    SMSGuiElement elements;
    elements.lblName = new QLabel (entry.strName, this);
    elements.lblNumberInfo = new QLabel (strNumberinfo, this);
    elements.btnDelete = new SMSEntryDeleteButton (index, this);

    QObject::connect (elements.btnDelete, SIGNAL (triggered (int)),
                      this              , SLOT   (btnDelClicked (int)));

    smsGuiEntries += elements;
    smsEntries += entry;

    repopulateGrid ();
    return (true);
}//SMSDlg::addSMSEntry

void
SMSDlg::btnDelClicked (int index)
{
    delEntry (index);
    repopulateGrid ();
}//SMSDlg::btnDelClicked

void
SMSDlg::delEntry (int index)
{
    SMSGuiElement element = smsGuiEntries[index];
    smsGuiEntries.remove (index);
    smsEntries.remove (index);

    delete element.lblName;
    delete element.lblNumberInfo;
    delete element.btnDelete;
}//SMSDlg::delEntry

void
SMSDlg::repopulateGrid ()
{
    if (NULL != this->layout ())
    {
        delete this->layout ();
    }

    QGridLayout *grid = new QGridLayout (this);

    int nRowCount = 0;
    for (; nRowCount < smsGuiEntries.size (); nRowCount++)
    {
        grid->addWidget (smsGuiEntries[nRowCount].lblName, nRowCount, 0);
        grid->addWidget (smsGuiEntries[nRowCount].lblNumberInfo, nRowCount, 1);
        grid->addWidget (smsGuiEntries[nRowCount].btnDelete, nRowCount, 2);
        smsGuiEntries[nRowCount].btnDelete->setIndex (nRowCount);
    }

    grid->addWidget (&edSMSText   , nRowCount++,0, 1,3);
    grid->addWidget (&lblTextCount, nRowCount  ,0, 1,2);
    grid->addWidget (&btnSendSMS  , nRowCount  ,2);
    this->setLayout (grid);

    if (1 == nRowCount)
    {
        this->hide ();
    }
}//SMSDlg::repopulateGrid

void
SMSDlg::btnSendSMSClicked ()
{
    QStringList arrNumbers;
    for (int i = 0; i < smsEntries.size(); i++)
    {
        arrNumbers += smsEntries[i].sNumber.strNumber;
        delEntry (i);
    }
    repopulateGrid ();
    this->hide ();

    emit sendSMS (arrNumbers, edSMSText.toPlainText ());

    edSMSText.setPlainText ("");
}//SMSDlg::btnSendSMSClicked

void
SMSDlg::smsTextChanged ()
{
    QString strText = edSMSText.toPlainText ();

    int len = strText.size ();

    int smscount = 0, lastlen = 0;
    smscount = len / 140;
    lastlen  = len % 140;

    QString strLen = QString ("Remaining characters = %1").arg (140 - lastlen);
    if (0 != smscount)
    {
        smscount++; // Adjust to 1-base

        strLen += QString (" in the %1%2 SMS").arg (smscount);
        const char * suffix = NULL;
        switch (smscount)
        {
        case 1:
            suffix = "st";
            break;
        case 2:
            suffix = "nd";
            break;
        case 3:
            suffix = "rd";
            break;
        default:
            suffix = "th";
            break;
        }
        strLen = strLen.arg (suffix);
    }

    lblTextCount.setText (strLen);
}//SMSDlg::smsTextChanged
