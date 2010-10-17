import Qt 4.7

Rectangle {
    width: 640; height: 250

    Flow {
        anchors.fill: parent

        TextEdit {
            id: txtNum
            width: 200; height: 30
            text: "Enter a number"
            font { pointSize: 14; bold: true;  }
        }
        Keypad {}
    }
}
