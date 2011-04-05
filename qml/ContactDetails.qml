import Qt 4.7
import "helper.js" as Code

Flickable {
    id: container
    signal sigCall (string number)
    signal sigText (string number)

    property alias model: listview.model
    property alias notesText: notes.text
    property int suggestedPixelSize: 500

    ListView {  // All phone numbers for this contact
        id: listview
        width: parent.width
        anchors {
            top: parent.top
            left: parent.left
            topMargin: 4
        }
        height: (model ? ((container.suggestedPixelSize+3) * model.count * 2) : 1)
        spacing: 3
        clip: true

        delegate: Item { // one phone number
            id: delegateItem
            width: parent.width
            height: container.suggestedPixelSize * 2

            Text { // The phone number
                id: textNumber
                width: parent.width
                anchors {
                    top: parent.top
                    left: parent.left
                }

                text: type + " : " + number
                color: "white"
                font.pixelSize: (parent.height/2) - 6
            }// Item (phone number)

            Row {
                anchors {
                    top: textNumber.bottom
                    left: parent.left
                }
                width: parent.width
                height: parent.height / 2
                spacing: 2

                MyButton {
                    id: btnCall
                    mainText: "Call"
                    mainPixelSize: parent.height - 6
                    width: parent.width / 2
                    height: parent.height

                    onClicked: container.sigCall(number)
                }

                MyButton {
                    id: btnText
                    mainText: "Text"
                    mainPixelSize: parent.height - 6
                    width: parent.width / 2
                    height: parent.height

                    onClicked: container.sigText(number)
                }
            }// Row (Call and Text buttons)
        }// delegate Item (one phone number)
    }//ListView (All phone numbers for this contact)

    Text {
        id: notes
        text: "some notes"
        width: parent.width
        anchors {
            top: listview.bottom
            left: parent.left
            bottom: parent.bottom
        }

        wrapMode: Text.WordWrap

        font.pixelSize: (parent.height + parent.width) / 40
        color: "white"
    }// Text (contact notes)
}//Flickable (container)
