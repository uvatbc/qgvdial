/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2025 Yuvraaj Kelkar

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

HtmlFieldParser::HtmlFieldParser ()
: m_emitLog (true)
{
}//HtmlFieldParser::HtmlFieldParser

bool
HtmlFieldParser::parse (const QString &xmlData)
{
    elems.clear ();
    attrMap.clear ();

    QXmlStreamReader reader(xmlData);
    QString currentChars;

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            QString localName = reader.name().toString();
            currentChars.clear();

            QVariantMap aMap;
            const QXmlStreamAttributes &attrs = reader.attributes();
            for (int i = 0; i < attrs.count(); ++i) {
                aMap[attrs.at(i).name().toString()] = attrs.at(i).value().toString();
            }
            attrMap[localName] = aMap;
        } else if (token == QXmlStreamReader::Characters) {
            currentChars += reader.text().toString();
        } else if (token == QXmlStreamReader::EndElement) {
            elems[reader.name().toString()] = currentChars;
        }
    }

    if (reader.hasError()) {
        if (m_emitLog) {
            Q_WARN(QString("HtmlFieldParser XML error: %1").arg(reader.errorString()));
        }
        return false;
    }

    return true;
}//HtmlFieldParser::parse

void
HtmlFieldParser::setEmitLog (bool enable)
{
    m_emitLog = enable;
}//HtmlFieldParser::setEmitLog
