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

import Qt 4.7
import "meego"
import "generic"
import "s3"

Item {
    id: container

    signal sigLinkActivated(string strLink)

    height: mainColumn.height

    Column {
        id: mainColumn
        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 2
        width: parent.width

        QGVLabel {
            text: "Version: __QGVDIAL_VERSION__"
            width: parent.width
        }//QGVLabel (version)

        QGVLabel {
            width: parent.width

            onLinkActivated: container.sigLinkActivated(link)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            text: "Project <a href=http://www.code.google.com/p/qgvdial>homepage</a>, " +
                  "<a href=http://www.code.google.com/p/qgvdial/wiki/Changelog>Changelog</a> and " +
                  "<a href=http://www.code.google.com/p/qgvdial/w/list>Wiki</a> and " +
                  "<a href=http://yuvraaj.net/qgvdial/privacy.html>privacy policy</a>"
        }//QGVLabel (links)
    }//Column
}//Item(container)
