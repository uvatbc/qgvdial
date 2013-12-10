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

import QtQuick 1.1
import com.nokia.symbian 1.1

Item {
    id: container

    height: mainColumn.height + 2

    Column {
        id: mainColumn

        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 5
        width: parent.width

        CheckBox {
            id: optContactsUpdate
            objectName: "OptContactsUpdate"
            text: "Update contacts"
        }
        TextField {
            id: edContactsUpdateFreq
            objectName: "EdContactsUpdateFreq"
            visible: optContactsUpdate.checked
            validator: IntValidator { bottom: 1; top: 1440 }
        }

        CheckBox {
            id: optInboxUpdate
            objectName: "OptInboxUpdate"
            text: "Update contacts"
        }
        TextField {
            id: edInboxUpdateFreq
            objectName: "EdInboxUpdateFreq"
            visible: optInboxUpdate.checked
            validator: IntValidator { bottom: 1; top: 1440 }
        }
    }// Column
}// Item (top level)
