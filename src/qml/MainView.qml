import Qt 4.7

Rectangle {
    width: 390; height: 170

    Flow {
        anchors.fill: parent

        DialDisp { id: wDisp }
        Keypad {
            onBtnClick: {
                wDisp.text += strText
            }
        }// Keypad
    }// Flow
}// Rectangle
