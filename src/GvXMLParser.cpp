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

#include "GvXMLParser.h"
#include <QtXmlPatterns>

GvXMLParser::GvXMLParser (QObject *parent)
: QObject (parent)
, bEmitLog (true)
{
}//GvXMLParser::GvXMLParser

bool
GvXMLParser::startElement (const QString        & /*namespaceURI*/,
                           const QString        & /*localName   */,
                           const QString        & /*qName       */,
                           const QXmlAttributes & /*atts        */)
{
    strChars.clear ();
    return (true);
}//GvXMLParser::startElement

bool
GvXMLParser::endElement (const QString & /*namespaceURI*/,
                         const QString &localName   ,
                         const QString & /*qName       */)
{
    if (localName == "json") {
        strJson = strChars;

#if 0
        QFile temp("dump-json.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (strJson.toAscii ());
        temp.close ();
#endif
    }

    if (localName == "html") {
        strHtml = "<html>" + strChars.trimmed ()+ "</html>";

#if 0
        QFile temp("dump-html.txt");
        temp.open (QIODevice::ReadWrite);
        temp.write (strHtml.toAscii ());
        temp.close ();
#endif
    }// if html
    return (true);
}//GvXMLParser::endElement

bool
GvXMLParser::characters (const QString &ch)
{
    strChars += ch;
    return (true);
}//GvXMLParser::characters

void
GvXMLParser::setEmitLog (bool enable)
{
    bEmitLog = enable;
}//GvXMLParser::setEmitLog
