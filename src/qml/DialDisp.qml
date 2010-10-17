import Qt 4.7

Rectangle {
    id: wDisp
    width: 200; height: 30
    property string text: ""

    Column {
        anchors.fill: parent

        ComboBoxPhones {
            lstItems: ["a", "b"]
            currSelected: 0
        }

        TextEdit {
            id: txtNum
            width: 200; height: 30
            textFormat: TextEdit.PlainText
            text: wDisp.text
            font { pointSize: 14; bold: true;  }
        }// TextEdit
    }// Flow
}// Rectangle
