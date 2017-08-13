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

    signal done(bool accepted, int selection)
    onDone: { g_mainwindow.onTfaMethodDlg(accepted, selection); }

    function clearModel() {
        optionsList.model.clear();
    }
    function setModel(option) {
        optionsList.model.append({name: option});
    }

    Label {
        id: topLabel
        text: "Select the TFA method"
        anchors {
            top: parent.top
        }

        color: "white"
        font.pixelSize: 45
    }

    ListView {
        id: optionsList

        width: parent.width
        anchors {
            top: topLabel.bottom
            topMargin: 5
            bottom: buttonRow.top
            bottomMargin: 5
        }

        clip: true

        model: ListModel {
            ListElement { name: "who" }
            ListElement { name: "why" }
        }

        delegate: Label {
            text: name
            color: "white"
            font.pixelSize: 40

            width: optionsList.width - 5

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: (index == optionsList.currentIndex ? "orange" : "transparent")
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var i;
                    for (i = 0; i < optionsList.model.count; i++) {
                        if (optionsList.model.get(i).name == text) {
                            optionsList.currentIndex = i;
                        }
                    }
                    console.log("Text = " + text + ", index = " + optionsList.currentIndex)
                }
            }
        }
    }

    ButtonRow {
        id: buttonRow

        width: parent.width * 8/10
        anchors {
            bottom: parent.bottom
            bottomMargin: 15
            horizontalCenter: parent.horizontalCenter
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

