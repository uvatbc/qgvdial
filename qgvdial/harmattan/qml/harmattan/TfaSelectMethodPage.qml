/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2017 Yuvraaj Kelkar

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
import com.nokia.meego 1.1

Page {
    id: container

    property variant tfaMethodModel: []
    property alias selectedIndex: optionsList.selectedIndex

    signal done(bool accepted, int selection)
    onDone: { g_mainwindow.onTfaMethodDlg(accepted, selection); }

    onTfaMethodModelChanged: {
        console.log("Model changed. len = " + optionsList.model.count)
    }

    SelectionDialog {
        id: optionsList

        model: container.tfaMethodModel

        width: parent.width
        anchors {
            top: parent.top
            bottom: buttonRow.top - 5
        }
    }//SelectionDialog

    ButtonRow {
        id: buttonRow

        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom - 15
        }

        exclusive: false
        spacing: 5

        Button {
            text: "Cancel"
            onClicked: { container.done(false, -1); }
        }
        Button {
            text: "Ok"
            onClicked: { container.done(true, container.selectedIndex); }
        }//Button
    }//ButtonRow
}//TFA Method Selection Dialog

