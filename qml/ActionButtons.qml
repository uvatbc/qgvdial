import Qt 4.7

Rectangle {
    signal sigCall
    signal sigText
    signal sigContacts
    signal sigInbox
    signal sigDel

    Row {
        anchors.fill: parent

        MyButton {
            id: btnCall
            mainText: "\u2706/\u270D"

            width: parent.width * (7 / 16)
            height: parent.height

            onClicked: sigCall()
            onPressHold: sigText()
        }

        MyButton {
            id: btnContacts
            mainText: "\u2625"

            width: parent.width * (3 / 16)
            height: parent.height

            onClicked: sigContacts()
        }

        MyButton {
            id: btnInbox
            mainText: "\u25D4"

            width: parent.width * (3 / 16)
            height: parent.height

            onClicked: sigInbox()
        }

        MyButton {
            id: btnDel
            mainText: "\u232B"

            width: parent.width * (3 / 16)
            height: parent.height

            onClicked: sigDel()
        }
    }
}
