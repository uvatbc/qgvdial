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

Rectangle {
    id: container
    color: "black"

    signal sigCall(string number)
    signal sigText(string name, string number)
    signal sigSearchContacts(string query)

    signal sigRefreshContacts
    signal sigRefreshAllContacts

    onSigRefreshContacts: console.debug("ping contacts")

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
            height: edSearch.height + 3
            width: parent.width
            spacing: 1

            property string lastSearchValue: ""
            function doSearch() {
                if (searchRow.lastSearchValue != edSearch.text) {
                    container.sigSearchContacts(edSearch.text);
                    searchRow.lastSearchValue = edSearch.text;
                }
                edSearch.closeSoftwareInputPanel();

                if (edSearch.text != "") {
                    imgSearch.selection = true;
                } else {
                    imgSearch.selection = false;
                }
            }

            Timer {
                id: searchTimeout
                interval: (1.2 * 1000)
                repeat: false

                onTriggered: searchRow.doSearch();
            }

            QGVLabel {
                id: searchLabel
                text: "Search:"
                fontPointMultiplier: (6.0/8.0)
                anchors.verticalCenter: parent.verticalCenter
            }//QGVLabel ("Search")

            MyTextEdit {
                id: edSearch
                width: parent.width - imgSearch.width - searchLabel.width - (parent.spacing * 2)
                anchors.verticalCenter: parent.verticalCenter
                pointSize: 10 * g_fontMul
                text: ""
                onTextChanged: {
                    if (imgSearch.selection) {
                        imgSearch.selection = false;
                    }

                    searchTimeout.stop();
                    searchTimeout.start();
                }

                onSigEnter: {
                    searchRow.doSearch();
                }
            }//MyTextEdit (search box text edit)

            Image {
                id: imgSearch
                source: (imgSearch.selection ? "close.png" : "search.png")
                height: edSearch.height * 0.75
                width: height

                anchors.verticalCenter: parent.verticalCenter

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
        }//Row (Search box)

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

            header: ListRefreshComponent {
                width: contactsView.width
                copyY: contactsView.contentY

                onClicked: container.sigRefreshContacts();
                onPressAndHold: container.sigRefreshAllContacts();
            }

            model: imgSearch.selection ? g_contactsSearchModel : g_contactsModel
//            model: testContactsModelData1

            section.property: "name"
            section.criteria: ViewSection.FirstCharacter

            delegate: Rectangle {
                id: listDelegate

                color: "#202020"
                border.color: "darkslategray"
                radius: 5

                width:  (allContacts.width - border.width)
                height: contactImage.height + (2 * g_hMul)

                Row {
                    anchors {
                        left: parent.left
                        leftMargin: 1 * g_wMul
                        right: parent.right
                        rightMargin: 1 * g_wMul
                    }

                    spacing: 2 * g_wMul

                    Image {
                        id: contactImage
                        anchors.verticalCenter: parent.verticalCenter

                        height: 24 * g_hMul
                        width: height

                        source: imagePath ? imagePath : "unknown_contact.png"
                        smooth: true
                    }//Image (contact images)

                    QGVLabel {
                        id: contactNameText

                        anchors {
                            verticalCenter: parent.verticalCenter
                        }
                        //width: parent.width - contactImage.width

                        text: name
                        fontPointMultiplier: (10.0/8.0)
                    }//Text (contact name)
                }// Row (image and contact name)

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        contactDetails.model = contacts;
                        contactDetails.notesText = notes;
                        contactDetails.name = name;
                        contactDetails.imageSource = (imagePath ? imagePath : "unknown_contact.png");
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
