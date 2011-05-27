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

import Qt 4.7
import "helper.js" as Code

Item {
    id: container

    signal sigBack()
    signal sigLinkActivated(string strLink)

    Column {
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 2
        spacing: 2

        property int pixDiv: 15
        property int pixHeight: (container.height + container.width) / 2
        property int pixSize: pixHeight / pixDiv

        Text {
            text: "Version: __QGVDIAL_VERSION__"
            width: parent.width
            font.pixelSize: mainColumn.pixSize
            color: "white"
        }//Text (version)

        Text {
            text: "<a href=http://www.code.google.com/p/qgvdial>Project homepage</a>"
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: mainColumn.pixSize
            color: "white"
            onLinkActivated: container.sigLinkActivated(link)
        }//Text (homepage)

        Text {
            text: "<a href=http://code.google.com/p/qgvdial/wiki/Changelog>Changelog</a>"
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: mainColumn.pixSize
            color: "white"
            onLinkActivated: container.sigLinkActivated(link)
        }//Text (Changelog)

        Text {
            text: "<a href=http://code.google.com/p/qgvdial/w/list>Wiki</a>"
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: mainColumn.pixSize
            color: "white"
            onLinkActivated: container.sigLinkActivated(link)
        }//Text (wiki)

        MyButton {
            mainText: "Back"
            width: parent.width
            mainPixelSize: mainColumn.pixSize

            onClicked: container.sigBack();
        }//MyButton (Back)
    }//Column

}//Item(container)
