import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: container
    width: 250; height: 320
    color: "black"

    signal sigCall(string number)
    signal sigText(string number)

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

    Rectangle {
        id: detailsView

        anchors.fill: parent
        color: "darkslategray"
        border.color: "orange"
        radius: 10

        opacity: 0

        Item {
            id: detailTopRow

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: btnDetailsClose.height

            Text {
                id: txtContactName

                text: "Contact name"
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                font.pointSize: Code.btnFontPoint () / 8
                anchors.left: parent.left
            }

            TextButton {
                id: btnDetailsClose
                text: "Close"
                onClicked: container.state= ''
                anchors.right: parent.right

                fontPoint: Code.btnFontPoint() / 8
            }
        }// Item (details top row)

        ContactDetails {
            id: detailsList
            anchors {
                top: detailTopRow.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            onSigCall: container.sigCall(number)
            onSigText: container.sigText(number)
        }
    }

    ListView {
        id: contactsView

        anchors.fill: parent
        clip: true
        opacity: 1

        model: g_contactsModel
//        model: testContactsModelData1

        delegate: Rectangle {
            id: listDelegate

            color: "darkslategray"
            border.color: "orange"
            radius: 5

            width: contactsView.width - border.width
            height: textName.height + 8

            Text {
                id: textName

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: 5
                }

                text: name
                color: "white"

                font.pointSize: (Code.btnFontPoint () / 8)
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    detailsList.model = contacts;
                    txtContactName.text = name
                    container.state = "Details"
                }
            }
        }// delegate Rectangle
    }// ListView

    states: [
        State {
            name: "Details"
            PropertyChanges { target: contactsView; opacity: 0 }
            PropertyChanges { target: detailsView; opacity: 1 }
        }
    ]

    transitions: [
        Transition {
            PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
        }
    ]

}// Rectangle
