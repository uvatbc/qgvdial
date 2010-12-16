import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: container
    width: 250; height: 320
    color: "black"

    signal sigCall(string strNumber)
    signal sigText(string strNumber)

    property bool landscape: container.width > container.height
    property variant rotationDelta: landscape? -90 : 0
    rotation: rotationDelta

/*
    ListModel {
        id: contactsModel

        ListElement {
            name: "Uv"
            contacts: [
                ListElement {
                    type: "Mobile"
                    number: "+1 408 111 2222"
                },
                ListElement {
                    type: "Work"
                    number: "+1 408 333 4444"
                },
                ListElement {
                    type: "Home"
                    number: "+1 408 555 6666"
                }
            ]
        }
        ListElement {
            name: "Yasho"
            contacts: [
                ListElement {
                    type: "Mobile"
                    number: "+1 408 777 8888"
                },
                ListElement {
                    type: "Work"
                    number: "+1 408 999 0000"
                }
            ]
        }
    }
*/

    ListView {
        id: listView
        model: contactsModel
        anchors.fill: parent

        delegate: Item {
            id: listDelegate
            property real detailsOpacity: 0

            width: listView.width
            height: {
                if (detailsOpacity == 0) {
                    return (listView.height / 5);
                } else {
                    return listView.height / 3;
                }
            }

            MouseArea {
                anchors.fill: parent

                onClicked: listDelegate.state = 'Details'
            }

            // Bounding rectangle
            Rectangle {
                anchors.fill: parent
                color: "darkslategray"
                border.color: "orange"
                radius: 10
                opacity: 1
            }

            // Topmost information in each delegate entry
            Item {
                id: topRow

                anchors.left: parent.left
                width: parent.width
                height: Math.max(textName.height, btnClose.height) + 10

                Row {
                    anchors.fill: parent

                    Text {
                        id: textName
                        width: parent.width - btnClose.width

                        text: model.name
                        color: "white"

                        anchors {
                            top: parent.top
                            topMargin: 10
                            leftMargin: 3
                        }

                        font.pointSize: (Code.btnFontPoint () / 8)
                    }

                    TextButton {
                        id: btnClose

                        text: "Close"
                        fontPoint: (Code.btnFontPoint () / 12)

                        opacity: listDelegate.detailsOpacity
                        anchors {
                            top: parent.top
                            topMargin: 10
                        }

                        onClicked: listDelegate.state = ''
                    }
                }
            }

            // Details in the delegate
            Item {
                id: bigForm
                opacity: listDelegate.detailsOpacity
                anchors {
                    top: topRow.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }

                width: parent.width
                height: (parent.height - topRow.height)

                ContactDetails {
                    id: listPhones
                    model: contacts
                    anchors.fill: parent
                    spacing: 2
                    clip: true

                    onSigCall: container.sigCall (number)
                    onSigText: container.sigText (number)
                }
            }

            states: State {
                name: "Details"
                PropertyChanges { target: listDelegate; detailsOpacity: 1 }

                // Move the list so that this item is at the top.
                PropertyChanges { target: listDelegate.ListView.view; explicit: true; contentY: listDelegate.y }

                // Disallow flicking while we're in detailed view
                PropertyChanges { target: listDelegate.ListView.view; interactive: false }
            }

            transitions: Transition {
                // Make the state changes smooth
                ParallelAnimation {
//                    ColorAnimation { property: "color"; duration: 500 }
                    NumberAnimation { duration: 300; properties: "detailsOpacity,x,height,width" }
                }
            }
        }// delegate Rectangle
    }// ListView
}// Rectangle
