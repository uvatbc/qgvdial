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

Rectangle {
    id: container
    color: "black"

    signal sigCall(string number)
    signal sigText(string name, string number)
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

    ContactDetails {
        id: contactDetails
        anchors.fill: parent
        opacity: 0

        onSigCall: container.sigCall(number)
        onSigText: container.sigText(name, number)

        onSigClose: container.state= '';
    }//ContactDetails

    Item { // All contacts
        id: allContacts
        anchors.fill: parent

        Row {
            id: searchRow
            height: (allContacts.height + allContacts.width) / 20
            width: parent.width
            spacing: 1

            property string lastSearchValue: ""
            function doSearch() {
                if (searchRow.lastSearchValue != edSearch.text) {
                    container.sigSearchContacts(edSearch.text);
                    searchRow.lastSearchValue = edSearch.text;
                }
                if (edSearch.text != "") {
                    imgSearch.selection = true;
                } else {
                    imgSearch.selection = false;
                }
            }

            Image {
                id: imgSearch
                source: (imgSearch.selection ? "close.png" : "search.png")
                height: searchRow.height
                width: searchRow.height
                fillMode: Image.PreserveAspectFit

                property bool selection: false

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (imgSearch.selection) {
                            edSearch.text = "";
                        }
                        searchRow.doSearch();
                    }
                }
            }//Image (search or close button)

            MyTextEdit {
                id: edSearch
                width: parent.width - imgSearch.width - parent.spacing
                pixelSize: searchRow.height - 6
                text: ""
                onTextChanged: {
                    if (imgSearch.selection) {
                        imgSearch.selection = false;
                    }
                }

                onSigEnter: {
                    searchRow.doSearch();
                }
            }//MyTextEdit (search box text edit)
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
            cacheBuffer: (100 * (allContacts.height + allContacts.width))

            model: g_contactsModel
//            model: testContactsModelData1

            section.property: "name"
            section.criteria: ViewSection.FirstCharacter

            delegate: Rectangle {
                id: listDelegate

                color: "#202020"
                border.color: "darkslategray"
                radius: 5

                width:  (allContacts.width - border.width)
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
                        contactDetails.model = contacts;
                        contactDetails.notesText = notes;
                        contactDetails.name = name;
                        contactDetails.imageSource = imagePath;
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
    }// Item (Search box, all contacts and the scroll bar)

    states: [
        State {
            name: "Details"
            PropertyChanges { target: allContacts; opacity: 0 }
            PropertyChanges { target: contactDetails; opacity: 1 }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
        }
    ]

}// Rectangle
