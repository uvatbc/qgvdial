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

    ListView { // All contacts
        id: contactsView

        anchors.fill: parent
        clip: true
        opacity: 1
        spacing: 2

        model: g_contactsModel
//        model: testContactsModelData1

        delegate: Rectangle {
            id: listDelegate

            color: "darkslategray"
            border.color: "orange"
            radius: 5

            width: contactsView.width - border.width
            height: (contactsView.height + contactsView.width) / 20

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
                    txtContactName.text = name
                    container.state = "Details"
                }
            }
        }// delegate Rectangle
    }// ListView (All contacts)

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
