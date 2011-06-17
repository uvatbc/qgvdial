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

Rectangle {
    id: container
    color: "black"

    signal sigCall(string number)
    signal sigText(string number)
    signal sigSearchContacts(string query)

////////////////////////////////////////////////////////////////////////////////
//                              Test Data models                              //
////////////////////////////////////////////////////////////////////////////////
//    ContactsModelData1 {
//        id: testContactsModelData1
//    }

//    XmlListModel {
//        id: testContactsModelData2
//        source: "./ContactsModelData2.xml"
//        query: "/all_contacts/one_contact"

//        XmlRole { name: "name"; query: "@name/string()" }
//        XmlRole { name: "contacts"; query: "contact/*" }
//    }
////////////////////////////////////////////////////////////////////////////////

    Rectangle { // Contact details
        id: detailsView

        anchors.fill: parent
        color: "#202020"
        border.color: "orange"
        radius: 10

        opacity: 0

        Column {
            id: detailTopRow

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                leftMargin: 2
                rightMargin: 2
            }
            spacing: 2

            MyButton {
                id: detailsCloseButton
                mainText: "Close"
                onClicked: container.state= ''
                width: parent.width
                height: txtContactName.height
                mainPixelSize: height - 4
            }

            Item {
                height: contactDetailImage.height
                width: parent.width

                Image {
                    id: contactDetailImage
                    anchors.left: parent.left
                    height: (txtContactName.height * 2.5)
                    width: (txtContactName.height * 2.5)

                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    id: txtContactName

                    anchors {
                        left: contactDetailImage.right
                        verticalCenter: parent.verticalCenter
                    }
                    width: parent.width - contactDetailImage.width

                    text: "Contact name"
                    color: "white"
                    font.pixelSize: (detailsView.width + detailsView.height) / 30
                }
            }
        }// Column (details top row)

        ContactDetails {
            id: detailsList
            anchors {
                top: detailTopRow.bottom
                bottom: parent.bottom
                left: parent.left
                topMargin: 5
            }
            width:  parent.width
            suggestedPixelSize: (parent.width + parent.height) / 30

            onSigCall: container.sigCall(number)
            onSigText: container.sigText(number)
        }
    }// Rectangle (Contact details)

    Item { // All contacts
        id: allContacts
        anchors.fill: parent

        Row {
            id: searchRow
            height: (allContacts.height + allContacts.width) / 20
            width: parent.width
            spacing: 1

            Image {
                id: imgSearch
                source: (edSearch.text.length == 0 ? "search.png" : "close.png")
                height: searchRow.height
                width: searchRow.height
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    onClicked: edSearch.text = ""
                }
            }

            MyTextEdit {
                id: edSearch
                width: parent.width - imgSearch.width - parent.spacing
                pixelSize: searchRow.height - 6
                text: ""
                onSigTextChanged: container.sigSearchContacts(strText)
            }
        }//Search box

        ListView {
            id: contactsView

            anchors {
                top: searchRow.bottom
                topMargin: 2
                left: parent.left
            }
            height: parent.height - searchRow.height
            width: parent.width

            clip: true
            opacity: 1
            spacing: 2

            model: g_contactsModel
    //        model: testContactsModelData1

            section.property: "name"
            section.criteria: ViewSection.FirstCharacter

            delegate: Rectangle {
                id: listDelegate

                color: "#202020"
                border.color: "darkslategray"
                radius: 5

                width: allContacts.width - border.width
                height: (allContacts.height + allContacts.width) / 20

                Image {
                    id: contactImage
                    anchors {
                        left: parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    height: parent.height
                    width: parent.height

                    source: imagePath
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: contactImage.right
                        leftMargin: 3
                    }
                    width: parent.width - contactImage.width

                    text: name
                    color: "white"

                    font.pixelSize: parent.height - 6
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        detailsList.model = contacts;
                        detailsList.notesText = notes;
                        txtContactName.text = name;
                        contactDetailImage.source = imagePath;
                        container.state = "Details";
                    }
                }
            }// delegate Rectangle
        }// ListView (contacts list)

        Scrollbar {
            scrollArea: contactsView
            width: 8
            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
        }//scroll bar for the contacts list
    }// Item (All contacts)

    Rectangle {
        opacity: contactsView.moving ? 0.5 : 0
        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
        }

        height: container.height/5
        width: container.width/5
        color: "black"
        border.color: "green"

        Text {
            text: contactsView.currentSection
            font.pixelSize: parent.height / 2
            anchors.centerIn: parent

            color: "white"
        }
    }// The first letter view

    states: [
        State {
            name: "Details"
            PropertyChanges { target: allContacts; opacity: 0 }
            PropertyChanges { target: detailsView; opacity: 1 }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
        }
    ]

}// Rectangle
