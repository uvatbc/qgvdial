import Qt 4.7
import "helper.js" as Code

ListView {
    id: container

    signal sigCall (string number)
    signal sigText (string number)

    delegate: Item {
        width: parent.width
        height: textNumber.height + btnCall.height

        Text {
            id: textNumber
            width: parent.width
            anchors {
                top: parent.top
                left: parent.left
            }

            text: type + "\t: " + number
            color: "white"
            font.pointSize: (Code.btnFontPoint () / 12)
        }

        Row {
            anchors {
                top: textNumber.bottom
                right: parent.right
            }

            TextButton {
                id: btnCall
                text: "Call"
                fontPoint: (Code.btnFontPoint () / 12)

                onClicked: container.sigCall(number)
            }

            TextButton {
                id: btnText
                text: "Text"
                fontPoint: (Code.btnFontPoint () / 12)

                onClicked: container.sigText(number)
            }
        }// Row (of buttons)
    }// delegate Item
}// main ListView
