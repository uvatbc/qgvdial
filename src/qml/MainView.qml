import Qt 4.7

Rectangle {
    id: wMainView
    color: "black"
    width: 250; height: 400

    Flow {
        anchors.fill: parent
        spacing: 2

        DialDisp {
            id: wDisp
            color: wMainView.color
        }//DialDisp

        Keypad {
            color: wMainView.color
            onBtnClick: {
                wDisp.strNum += strText
            }
        }//Keypad
    }//Flow
}//Rectangle
