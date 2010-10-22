import Qt 4.7
import "helper.js" as Code

Rectangle {
    id: wDisp
    // The number in the text box
    property string strNum: txtNum.text
    property string layoutName: "maemo-portrait"

    width: Code.calcFlowChildWidth();
    height: Code.calcFlowChildHeight();

    Column {
        anchors.fill: parent

        ComboBoxPhones {
            lstItems: ["a", "b"]
            currSelected: 0
            layoutName: wDisp.layoutName
        }

        TextEdit {
            id: txtNum
            color: "white"
            width: wDisp.width
            height: (wDisp.layoutName=="desktop"?30:60)
            textFormat: TextEdit.PlainText
            text: wDisp.strNum
            font {
                pointSize: (wDisp.layoutName=="desktop"?14:28)
                bold: true
            }
        }// TextEdit
    }// Flow
}// Rectangle
