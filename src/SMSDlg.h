#ifndef __SMSDLG_H__
#define __SMSDLG_H__

#include "global.h"
#include "ChildWindowBase.h"
#include "SMSEntryDeleteButton.h"

struct SMSEntry
{
    QString         strName;
    GVContactNumber sNumber;
};

struct SMSGuiElement
{
    QLabel               *lblName;
    QLabel               *lblNumberInfo;
    SMSEntryDeleteButton *btnDelete;
};

class SMSDlg : public ChildWindowBase
{
    Q_OBJECT

public:
    SMSDlg (QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~SMSDlg(void);

    bool addSMSEntry (const SMSEntry &entry);

signals:
    void sendSMS (const QStringList &arrNumbers, const QString &strText);

private slots:
    void btnDelClicked (int index);
    void btnSendSMSClicked ();
    void smsTextChanged ();

private:
    void delEntry (int index);
    void repopulateGrid ();

private:
    //! Contains the information needed to show numbers at the top.
    QVector<SMSEntry>       smsEntries;
    //! Contains the corresponding GUI elements to show the numbers
    QVector<SMSGuiElement>  smsGuiEntries;

    //! Edit box where the user will write the text message
    QPlainTextEdit  edSMSText;

    //! Label which hold the count of characters in the text message
    QLabel          lblTextCount;
    //! The button to send the SMS
    QPushButton     btnSendSMS;
};

#endif //__SMSDLG_H__
