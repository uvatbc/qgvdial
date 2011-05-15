import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: container
    color: "black"

    signal sigCall(string number)
    signal sigText(string number)
    signal sigMsgBoxDone (bool ok)
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
        color: "darkslategray"
        border.color: "orange"
        radius: 10

        opacity: 0

        Column {
            id: detailTopRow

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: (parent.width + parent.height) / 15
            spacing: 2

            MyButton {
                mainText: "Close"
                onClicked: container.state= ''
                width: parent.width
                height: (parent.height / 2)
                mainPixelSize: height - 4
            }

            Text {
                id: txtContactName

                text: "Contact name"
                width: parent.width
                color: "white"
                font.pixelSize: (parent.height / 2) - 4
                anchors.left: parent.left
            }
        }// Column (details top row)

        ContactDetails {
            id: detailsList
            anchors {
                top: detailTopRow.bottom
                left: parent.left
                topMargin: 5
            }
            width:  parent.width
            height: parent.height - detailTopRow.height - 6
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
            height: lblSearch.height
            width: parent.width
            spacing: 1

            Text {
                id: lblSearch
                text: "?"
                color: "white"
                font.pixelSize: ((allContacts.height + allContacts.width) / 21)
            }

            MyTextEdit {
                id: edSearch
                width: parent.width - lblSearch.width - parent.spacing
                pixelSize: lblSearch.font.pixelSize
                text: ""
                onSigTextChanged: container.sigSearchContacts(strText)
            }
        }

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

                color: "darkslategray"
                border.color: "orange"
                radius: 5

                width: allContacts.width - border.width
                height: (allContacts.height + allContacts.width) / 20

                Text {
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 5
                    }

                    text: name
                    color: "white"

                    font.pixelSize: parent.height - 6
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        detailsList.model = contacts;
                        detailsList.notesText = notes;
                        txtContactName.text = name
                        container.state = "Details"
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

    MsgBox {
        id: msgBox
        opacity: ((container.opacity == 1 && g_bShowMsg == true) ? 1 : 0)
        msgText: g_strMsgText

        width: container.width - 20
        height: (container.width + container.height) / 6
        anchors.centerIn: container

        onSigMsgBoxOk: container.sigMsgBoxDone(true)
        onSigMsgBoxCancel: container.sigMsgBoxDone(false)
    }

}// Rectangle
