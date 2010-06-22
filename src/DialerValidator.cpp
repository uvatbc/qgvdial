#include "DialerValidator.h"

DialerValidator::DialerValidator (QObject *parent)
: QValidator(parent)
{
}//DialerValidator::DialerValidator

QValidator::State
DialerValidator::validate (QString &input, int &pos) const
{
    QString strNum = input.simplified ();
    if (0 == strNum.size ())
    {
        input = strNum;
        return (QValidator::Acceptable);
    }

    bool bHadPlus = false;
    if (strNum.startsWith ('+'))
    {
        strNum = strNum.mid (1);
        bHadPlus = true;
    }

    QString strTemp = strNum;
    strTemp.remove (QRegExp ("[0-9]+"));
    strTemp.remove (QRegExp ("[\\t\\n\\v\\f\\r ]+"));
    strTemp.remove (QRegExp ("[\\(\\)]+"));
    strTemp.remove (QRegExp ("[-]+"));
    if (0 != strTemp.size ())
    {
        strTemp = "[" + strTemp + "]+";
        strNum.remove (QRegExp (strTemp));
    }

    strTemp = strNum;
    strTemp.remove (QRegExp ("[\\t\\n\\v\\f\\r ]+"));
    strTemp.remove (QRegExp ("[\\(\\)]+"));
    strTemp.remove (QRegExp ("[-]+"));
    // if this is an american number... may be we can try fixing it
    if ((bHadPlus) && (strTemp.startsWith ('1')))
    {
        input  = "+1-";
        input += strTemp.mid (1, 3);
        input += "-";
        input += strTemp.mid (4, 3);
        input += "-";
        input += strTemp.mid (7);
    }
    else
    {
        if (bHadPlus)
        {
            input = "+";
        }
        else
        {
            input.clear ();
        }
        input += strNum.simplified ();
    }
    pos = input.size ();

    return (QValidator::Acceptable);
}//DialerValidator::validate
