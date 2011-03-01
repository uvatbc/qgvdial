import Qt 4.7
import "helper.js" as Code

Flickable {
    id: container
    signal sigCall (string number)
    signal sigText (string number)

    property alias model: listview.model
    property alias notesText: notes.text

    ListView {
        id: listview
        width: parent.width
        anchors {
            top: parent.top
            left: parent.left
        }
        height: (model ? (textCalc.height * model.count * 2.5) : 1)

        Text {
            id: textCalc
            opacity: 0
            text: "text"
            font.pointSize: (Code.btnFontPoint() / 12)
        }

        delegate: Item {
            id: delegateItem
            width: parent.width
            height: textNumber.height + btnCall.height

            Text {
                id: textNumber
                width: parent.width
                anchors {
                    top: parent.top
                    left: parent.left
                }

                text: type + " : " + number
                color: "white"
                font.pointSize: (Code.btnFontPoint() / 12)
            }

            Row {
                anchors {
                    top: textNumber.bottom
                    right: parent.right
                }

                TextButton {
                    id: btnCall
                    text: "Call"
                    fontPoint: (Code.btnFontPoint() / 12)

                    onClicked: container.sigCall(number)
                }

                TextButton {
                    id: btnText
                    text: "Text"
                    fontPoint: (Code.btnFontPoint() / 12)

                    onClicked: container.sigText(number)
                }
            }// Row (of buttons)
        }// delegate Item
    }//ListView

    Text {
        id: notes
        text: "some notes"
        width: parent.width
        anchors {
            top: listview.bottom
            left: parent.left
            bottom: parent.bottom
        }

        font.pointSize: Code.btnFontPoint()/12
        color: "white"
    }
}//Flickable (container)
