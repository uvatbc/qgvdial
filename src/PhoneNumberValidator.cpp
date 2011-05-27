/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2010  Yuvraaj Kelkar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Contact: yuvraaj@gmail.com
*/

#include "PhoneNumberValidator.h"

PhoneNumberValidator::PhoneNumberValidator (QObject *parent)
: QValidator(parent)
{
}//DialerValidator::DialerValidator

QValidator::State
PhoneNumberValidator::validate (QString &input, int &pos) const
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
