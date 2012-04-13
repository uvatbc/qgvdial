/*
qgvdial is a cross platform Google Voice Dialer
Copyright (C) 2009-2012  Yuvraaj Kelkar

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

Item {
    id: container

    signal sigDone(bool bSave)

    height: mainColumn.height + 2

    Column {
        id: mainColumn

        anchors {
            top: parent.top
            left: parent.left
        }
        spacing: 2
        width: parent.width
        height: childrenRect.height

        ListView {
            id: registeredPhonesView
            objectName: "RegisteredPhonesView"

            signal sigSelChanged (int index)
            function setSelected(index) {
                var i;
                for (i = 0; i < regPhoneModel.count; i++) {
                    regPhoneModel.setProperty (i, "isChecked", (i == index));
                }
            }

            RadioButton {
                id: delegateRadioButton
                visible: false
                width: parent.width
                text: "whatever"
            }

            width: parent.width
            height: (delegateRadioButton.height + spacing) * regPhoneModel.count
            interactive: false

            model: ListModel {
                id: regPhoneModel
                objectName: "RegisteredPhonesModel"
            }//ListModel (of the registered phones)

            delegate: RadioButton {
                width: parent.width
                text: entryText
                check: isChecked
                onClicked: {
                    registeredPhonesView.sigSelChanged(index);
                    registeredPhonesView.setSelected(index);
                }
            }
        }//ListView (of the registered phones)

        SaveCancel {
            anchors {
                left: parent.left
                leftMargin: 1
            }
            width: parent.width - 1

            onSigSave: {
                container.sigDone(true);
            }

            onSigCancel: container.sigDone(false);
        }// Save and cancel buttons
    }// Main
}//Item(container)
