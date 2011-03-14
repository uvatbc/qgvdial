import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: wDisp

    signal sigSelChanged (int index)
    signal sigNumChanged (string strNumber)

    // Expose the text edit as a property
    property alias txtEd: txtNum
    property alias theNumber: txtNum.text

    Item {
        id: mainItem
        anchors.fill: parent

        Rectangle {
            id: btnPhones

            color: wDisp.color
            width: parent.width
            height: (parent.height / 5)
            anchors {
                top: parent.top
                left: parent.left
            }

            MyButton {
                mainText: g_CurrentPhoneName;
                anchors.fill: parent
                radius: ((height / 10.0) + (width / 60.0))
                mainPixelSize: parent.height - 8

                onClicked: mainItem.state == "PhonesShown" ?
                           mainItem.state = "" : mainItem.state = "PhonesShown"
            }// MyButton (btnPhones)
        }// Rectangle (wDisp)

        TextEdit {
            id: txtNum
            opacity: 1

            width: parent.width
            height: parent.height - btnPhones.height
            anchors {
                top: btnPhones.bottom
                left: parent.left
            }

            color: "white"
            textFormat: TextEdit.PlainText
            cursorVisible: true
            wrapMode: TextEdit.WrapAnywhere
            selectByMouse: true
            font {
                pixelSize: height - 6
                bold: true
            }

            onTextChanged: wDisp.sigNumChanged(txtNum.text);
        }// TextEdit

        ComboBoxPhones {
            id: cbPhones
            opacity: 0

            width: parent.width
            anchors {
                top: btnPhones.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            onSelectionChanged: {
                wDisp.sigSelChanged(iIndex)
                mainItem.state =  ""
            }
        }

        states: [
            State {
                name: "PhonesShown"
                PropertyChanges { target: cbPhones; opacity: 1 }
                PropertyChanges { target: txtNum; opacity: 0 }
            }
        ]

        transitions: [
            Transition {
                PropertyAnimation { property: "opacity"; easing.type: Easing.InOutQuad}
            }
        ]
    }// Item
}// Rectangle
