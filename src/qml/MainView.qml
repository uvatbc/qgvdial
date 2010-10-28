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
                var origStart = wDisp.tEd.selectionStart;
                var result = wDisp.tEd.text.substr(0,origStart);
                result += strText;
                result += wDisp.tEd.text.substr(wDisp.tEd.selectionEnd);
                wDisp.tEd.text = result;
                wDisp.tEd.cursorPosition = origStart + strText.length;
            }
        }//Keypad
    }//Flow
}//Rectangle
