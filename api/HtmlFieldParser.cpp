/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2013  Yuvraaj Kelkar

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

#include "HtmlFieldParser.h"
#include <QtXmlPatterns>

HtmlFieldParser::HtmlFieldParser ()
: m_emitLog (true)
{
}//HtmlFieldParser::HtmlFieldParser

bool
HtmlFieldParser::startElement (const QString        & /*namespaceURI*/,
                               const QString        & localName       ,
                               const QString        & /*qName       */,
                               const QXmlAttributes & attrs           )
{
    strChars.clear ();

    QVariantMap aMap;
    for (int i = 0; i < attrs.count(); ++i) {
        aMap[attrs.localName(i)] = attrs.value(i);
    }
    attrMap[localName] = aMap;

    return (true);
}//HtmlFieldParser::startElement

bool
HtmlFieldParser::endElement (const QString & /*namespaceURI*/,
                             const QString &localName   ,
                             const QString & /*qName       */)
{
    elems[localName] = strChars;

#if 0
    QFile temp(QString("dump-%1.txt").arg(localName));
    temp.open (QIODevice::ReadWrite);
    temp.write (strChars.toAscii ());
    temp.close ();
#endif

    return (true);
}//HtmlFieldParser::endElement

bool
HtmlFieldParser::characters (const QString &ch)
{
    strChars += ch;
    return (true);
}//HtmlFieldParser::characters

void
HtmlFieldParser::setEmitLog (bool enable)
{
    m_emitLog = enable;
}//HtmlFieldParser::setEmitLog
